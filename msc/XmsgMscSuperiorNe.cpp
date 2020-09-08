/*
  Copyright 2019 www.dev5.cn, Inc. dev5@qq.com
 
  This file is part of X-MSG-IM.
 
  X-MSG-IM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  X-MSG-IM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU Affero General Public License
  along with X-MSG-IM.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "XmsgMscMgr.h"
#include "XmsgMscSuperiorNe.h"
#include "../XmsgMscCfg.h"

XmsgMscSuperiorNe::XmsgMscSuperiorNe(shared_ptr<XscTcpServer> tcpServer, const string &peer, const string& pwd, const string& alg, SptrCgt cgt) :
		XmsgImTcpH2N(tcpServer, peer)
{
	this->pwd = pwd;
	this->alg = alg;
	this->cgt = cgt;
}

void XmsgMscSuperiorNe::estab()
{
	LOG_INFO("got a connection from a superior x-msg-msc, cgt: %s, peer: %s", this->cgt->toString().c_str(), this->peer.c_str())
	this->setXscUsr(this->msc);
	shared_ptr<XmsgNeAuthReq> req(new XmsgNeAuthReq());
	req->set_redundant(XmsgNeRedundantType::LEADER_FOLLOWER);
	req->set_cgt(XmsgMscCfg::instance()->cfgPb->cgt());
	req->set_salt(Crypto::gen0aAkey256());
	req->set_sign(Crypto::sha256ToHexStrLowerCase(XmsgMscCfg::instance()->cfgPb->cgt() + req->salt() + this->pwd));
	req->set_alg(this->alg);
	auto msc = static_pointer_cast<XmsgMscSuperiorNe>(this->shared_from_this());
	this->begin(req, [req, msc](shared_ptr<XmsgImTransInitiative> trans)
	{
		if(trans->ret != RET_SUCCESS)
		{
			LOG_ERROR("auth with superior x-msg-msc failed, peer: %s, req: %s, ret: %04X, desc: %s, this: %s", msc->peer.c_str(), req->ShortDebugString().c_str(), trans->ret, trans->desc.c_str(), msc->toString().c_str())
			msc->close();
			return;
		}
		auto rsp = static_pointer_cast<XmsgNeAuthRsp>(trans->endMsg);
		SptrCgt cgt = ChannelGlobalTitle::parse(rsp->cgt());
		if(cgt == nullptr)
		{
			LOG_FAULT("superior x-msg-msc channel global title format error, rsp: %s", rsp->ShortDebugString().c_str())
			msc->close();
			return;
		}
		if(!cgt->isSame(msc->cgt))
		{
			LOG_FAULT("config superior x-msg-msc channel global title format error, cfg: %s, rsp: %s", msc->cgt->toString().c_str(), rsp->ShortDebugString().c_str())
			msc->close();
			return;
		}
		LOG_INFO("auth with superior x-msg-msc successful, channel global title: %s, peer: %s, req: %s, rsp: %s", msc->cgt->toString().c_str(), msc->peer.c_str(), req->ShortDebugString().c_str(), rsp->ShortDebugString().c_str())
	});
}

void XmsgMscSuperiorNe::evnDisc()
{
	LOG_ERROR("superior x-msg-msc channel lost: %s", this->toString().c_str())
	shared_ptr<XscUsr> usr = this->usr.lock();
	if (usr != nullptr)
	{
		usr->evnDisc();
		this->setXscUsr(nullptr);
	}
	this->connect();
}

XscMsgItcpRetType XmsgMscSuperiorNe::itcp(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)
{
	if (pdu->transm.header == NULL || pdu->transm.header->route == NULL) 
	{
		return XscMsgItcpRetType::DISABLE;
	}
	SptrCgt dest = ChannelGlobalTitle::parse(pdu->transm.header->route->dne); 
	if (dest == nullptr)
	{
		LOG_DEBUG("destination channel global title format error, sne: %s, dne: %s", pdu->transm.header->route->sne.c_str(), pdu->transm.header->route->dne.c_str())
		return XscMsgItcpRetType::FORBIDDEN;
	}
	if (!Misc::endWith(dest->domain, XmsgMscCfg::instance()->cgt->domain)) 
	{
		LOG_FAULT("it`s a bug, destination channel global title must be current msc or current msc`s subordinate, dest: %s, current-domain: %s", pdu->transm.header->route->dne.c_str(), XmsgMscCfg::instance()->cgt->domain.c_str())
		return XscMsgItcpRetType::FORBIDDEN;
	}
	if (dest->domain != XmsgMscCfg::instance()->cgt->domain) 
	{
		XmsgMscMgr::route2subordinate(channel, pdu, dest);
		return XscMsgItcpRetType::SUCCESS;
	}
	if (dest->hlr == XmsgMscCfg::instance()->cgt->hlr) 
	{
		if (dest->uid == XmsgMscCfg::instance()->cgt->uid) 
			return XscMsgItcpRetType::DISABLE;
		LOG_DEBUG("unsupported route message to partner in same group, self: %s, dest: %s", XmsgMscCfg::instance()->cgt->toString().c_str(), dest->toString().c_str())
		return XscMsgItcpRetType::FORBIDDEN; 
	}
	XmsgMscMgr::route2service(channel, pdu, dest);
	return XscMsgItcpRetType::SUCCESS;
}

string XmsgMscSuperiorNe::toString()
{
	string str;
	SPRINTF_STRING(&str, "cgt: %s, peer: %s", this->cgt->toString().c_str(), this->peer.c_str())
	return str;
}

XmsgMscSuperiorNe::~XmsgMscSuperiorNe()
{

}

