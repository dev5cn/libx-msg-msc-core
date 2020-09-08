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
#include "../XmsgMscCfg.h"

XmsgMscMgr* XmsgMscMgr::inst = new XmsgMscMgr();

XmsgMscMgr::XmsgMscMgr()
{

}

XmsgMscMgr* XmsgMscMgr::instance()
{
	return XmsgMscMgr::inst;
}

void XmsgMscMgr::addSuperior(shared_ptr<XmsgMscSuperior> superior)
{
	this->superior.push_back(superior);
	sort(this->superior.begin(), this->superior.end(), [](auto& a, auto&b) 
	{
		return a->uid < b->uid;
	});
}

shared_ptr<XmsgMscSuperior> XmsgMscMgr::chooseSuperior(SptrCgt cgt)
{
	if (this->superior.empty())
		return nullptr;
	int indx = (cgt->domain.data()[cgt->domain.length() - 1] & 0x00FF) % this->superior.size(); 
	return this->superior.at(indx);
}

void XmsgMscMgr::initSubordinate(const string& node, shared_ptr<vector<string>> cgt)
{
	sort(cgt->begin(), cgt->end(), [](auto& a, auto& b) 
	{
		return a < b;
	});
	shared_ptr<vector<pair<string, shared_ptr<XmsgMscSubordinate>>>> v(new vector<pair<string, shared_ptr<XmsgMscSubordinate>>>());
	for (size_t i = 0; i < cgt->size(); ++i)
		v->push_back(make_pair<>(cgt->at(i), nullptr ));
	this->subordinate[node] = v;
}

shared_ptr<XmsgMscSubordinate> XmsgMscMgr::addSubordinate(const string& node, shared_ptr<XmsgMscSubordinate> subordinate)
{
	auto it = this->subordinate.find(node);
	if (it == this->subordinate.end())
	{
		LOG_FAULT("it`s a bug, node: %s, subordinate: %s", node.c_str(), subordinate->toString().c_str())
		return nullptr;
	}
	shared_ptr<XmsgMscSubordinate> old = nullptr;
	auto v = it->second;
	unique_lock<mutex> lock(this->lock4subordinate);
	for (size_t i = 0; i < v->size(); ++i)
	{
		string cgt = subordinate->cgt->toString();
		if (cgt != v->at(i).first)
			continue;
		old = v->at(i).second; 
		v->at(i).second = subordinate;
	}
	return old;
}

shared_ptr<XmsgMscSubordinate> XmsgMscMgr::resetSubordinate(const string& node, SptrCgt cgt)
{
	auto it = this->subordinate.find(node);
	if (it == this->subordinate.end())
	{
		LOG_FAULT("it`s a bug, node: %s, subordinate: %s", node.c_str(), cgt->toString().c_str())
		return nullptr;
	}
	string c = cgt->toString();
	shared_ptr<XmsgMscSubordinate> sub = nullptr;
	auto v = it->second;
	unique_lock<mutex> lock(this->lock4subordinate);
	for (size_t i = 0; i < v->size(); ++i)
	{
		if (c != v->at(i).first)
			continue;
		sub = v->at(i).second;
		v->at(i).second = nullptr; 
	}
	return sub;
}

shared_ptr<XmsgMscSubordinate> XmsgMscMgr::chooseSubordinate(const string& node, SptrCgt dest)
{
	auto it = this->subordinate.find(node);
	if (it == this->subordinate.end())
	{
		LOG_DEBUG("can not found subordinate x-msg-msc group for node: %s, dest: %s", node.c_str(), dest->toString().c_str())
		return nullptr;
	}
	int indx = (dest->uid.data()[dest->uid.length() - 1] & 0x00FF) % it->second->size();
	return it->second->at(indx).second;
}

bool XmsgMscMgr::isSuperior(SptrCgt cgt)
{
	return this->isRoute2superior(cgt);
}

bool XmsgMscMgr::isSubordinate(SptrCgt cgt, string& node)
{
	if (!Misc::endWith(cgt->domain, "." + XmsgMscCfg::instance()->cgt->domain)) 
		return false;
	if (cgt->domain.length() == XmsgMscCfg::instance()->cgt->domain.length()) 
		return false;
	node = cgt->domain.substr(0, cgt->domain.length() - XmsgMscCfg::instance()->cgt->domain.length() - 1);
	return node.rfind(".") == string::npos; 
}

bool XmsgMscMgr::isRoute2superior(SptrCgt dest)
{
	string::size_type indx = dest->domain.find(XmsgMscCfg::instance()->cgt->domain);
	return (indx == string::npos || indx + XmsgMscCfg::instance()->cgt->domain.length() != dest->domain.length()); 
}

bool XmsgMscMgr::isRoute2subordinate(SptrCgt dest)
{
	string::size_type indx = dest->domain.find(XmsgMscCfg::instance()->cgt->domain);
	return (indx > 0 && indx + XmsgMscCfg::instance()->cgt->domain.length() == dest->domain.length()); 
}

void XmsgMscMgr::route2service(XscChannel* channel, shared_ptr<XscProtoPdu> pdu, SptrCgt dest)
{
	auto group = XmsgNeGroupMgr::instance()->findByGroupName(dest->domain + "." + dest->hlr); 
	if (group == nullptr)
	{
		XmsgMscMgr::routeFailed(channel, pdu, RET_ROUTE_FAILED, "can not found network element group for dest-domain-hlr");
		LOG_DEBUG("can not found network element group for domain-hlr, dest: %s", dest->toString().c_str())
		return;
	}
	auto ne = group->choose();
	if (ne == nullptr)
	{
		XmsgMscMgr::routeFailed(channel, pdu, RET_ROUTE_FAILED, "can not allocate network element for dest-domain-hlr");
		LOG_DEBUG("can not allocate network element for domain-hlr, may be all lost, dest: %s", dest->toString().c_str())
		return;
	}
	XmsgMscMgr::forward(ne, channel, pdu);
}

void XmsgMscMgr::route2subordinate(XscChannel* channel, shared_ptr<XscProtoPdu> pdu, SptrCgt dest)
{
	string node = dest->domain.substr(0, dest->domain.length() - XmsgMscCfg::instance()->cgt->domain.length() - 1);
	string::size_type indx = node.rfind(".");
	if (indx != string::npos) 
		node = node.substr(indx + 1, node.length());
	auto subordinate = XmsgMscMgr::instance()->chooseSubordinate(node, dest);
	if (subordinate == nullptr)
	{
		LOG_DEBUG("can not allocate a subordinate x-msg-msc for this message, dest: %s", dest->toString().c_str())
		return;
	}
	XmsgMscMgr::forward(subordinate, channel, pdu);
}

void XmsgMscMgr::route2superior(XscChannel* channel, shared_ptr<XscProtoPdu> pdu, SptrCgt dest)
{
	auto superior = XmsgMscMgr::instance()->chooseSuperior(dest);
	if (superior == nullptr)
	{
		LOG_WARN("can not allocate superior x-msg-msc for this message, dest: %s", dest->toString().c_str())
		return;
	}
	XmsgMscMgr::forward(superior, channel, pdu);
}

void XmsgMscMgr::forward(shared_ptr<XscUsr> usr, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)
{
	if (pdu->transm.trans->refDat && usr->wk != channel->wk) 
		pdu->transm.trans->cloneDat();
	usr->future([usr, pdu]
	{
		int len;
		uchar* dat = pdu->bytes(&len);
		usr->channel->send(dat, len);
		if (Log::isRecord())
		{
			bool exp;
			auto p = XscProtoPdu::decode(dat, len, &exp);
			LOG_RECORD("\n  --> PEER: %s CFD: %d NE: %s\n%s\n", usr->channel->peer.c_str(), usr->channel->cfd, usr->uid.c_str(), p == nullptr ? "exception" : p->print(dat, len).c_str())
		}
	});
}

void XmsgMscMgr::routeFailed(XscChannel* channel, shared_ptr<XscProtoPdu> pdu, ushort ret, const string& desc)
{
	if (pdu->transm.trans == NULL) 
		return;
	if (pdu->transm.trans->trans != XSC_TAG_TRANS_BEGIN) 
		return;
	shared_ptr<XscProtoPdu> ack(new XscProtoPdu());
	if (pdu->transm.header->trace != NULL) 
	{
		ack->transm.header = new xsc_transmission_header();
		ack->transm.header->trace = new xsc_transmission_header_trace();
		::memcpy(ack->transm.header->trace->tid, pdu->transm.header->trace->tid, sizeof(pdu->transm.header->trace->tid));
		::memcpy(ack->transm.header->trace->pid, pdu->transm.header->trace->pid, sizeof(pdu->transm.header->trace->pid));
		XscMisc::uuid(ack->transm.header->trace->sid);
		ack->transm.header->trace->sne = XmsgMscCfg::instance()->cgt->toString(); 
		ack->transm.header->trace->dne = pdu->transm.header->route->sne;
		ack->transm.header->trace->gts = DateMisc::nowGmt0();
	}
	if (ack->transm.header == NULL)
		ack->transm.header = new xsc_transmission_header();
	ack->transm.header->route = new xsc_transmission_header_route();
	ack->transm.header->route->sne = XmsgMscCfg::instance()->cgt->toString(); 
	ack->transm.header->route->dne = pdu->transm.header->route->sne;
	ack->transm.trans = new XscProtoTransaction();
	ack->transm.trans->trans = XSC_TAG_TRANS_END;
	ack->transm.trans->partSeq = 0x00;
	ack->transm.trans->haveNextPart = false;
	ack->transm.trans->refDat = true;
	ack->transm.trans->stid = 0x00;
	ack->transm.trans->dtid = pdu->transm.trans->stid;
	ack->transm.trans->ret = ret;
	ack->transm.trans->desc = desc;
	int len;
	uchar* dat = ack->bytes(&len);
	channel->send(dat, len);
	if (Log::isRecord())
	{
		auto usr = channel->usr.lock();
		bool exp;
		auto p = XscProtoPdu::decode(dat, len, &exp);
		LOG_RECORD("\n  --> PEER: %s CFD: %d NE: %s\n%s", channel->peer.c_str(), channel->cfd, usr == nullptr ? "" : usr->uid.c_str(), p == nullptr ? "exception" : p->print(dat, len).c_str())
	}
}

void XmsgMscMgr::subNeGroup(const string& cgt , const string& neg )
{
	unique_lock<mutex> lock(this->lock4negSub);
	auto it = this->subNeg.find(cgt);
	if (it == this->subNeg.end())
	{
		shared_ptr<unordered_set<string>> negs(new unordered_set<string>());
		negs->insert(neg);
		this->subNeg[cgt] = negs;
	} else
		it->second->insert(neg);
	auto iter = this->negSub.find(neg);
	if (iter == this->negSub.end())
	{
		shared_ptr<unordered_set<string>> subs(new unordered_set<string>());
		subs->insert(cgt);
		this->negSub[neg] = subs;
	} else
		iter->second->insert(cgt);
}

void XmsgMscMgr::removeSub(const string& cgt )
{
	unique_lock<mutex> lock(this->lock4negSub);
	this->subNeg.erase(cgt); 
	for (auto& it : this->negSub)
		it.second->erase(cgt); 
}

void XmsgMscMgr::xmsgNeUsrEvnEstab(shared_ptr<XmsgNeUsr> nu)
{
	XmsgMscMgr::instance()->pubXmsgNeUsrEvn(nu, true);
}

void XmsgMscMgr::xmsgNeUsrEvnDisc(shared_ptr<XmsgNeUsr> nu)
{
	SptrCgt cgt = ChannelGlobalTitle::parse(nu->uid);
	auto group = XmsgNeGroupMgr::instance()->findByGroupName(cgt->domain + "." + cgt->hlr); 
	group->remove(nu->uid); 
	XmsgMscMgr::instance()->removeSub(nu->uid); 
	XmsgMscMgr::instance()->pubXmsgNeUsrEvn(nu, false);
}

void XmsgMscMgr::pubXmsgNeUsrEvn(shared_ptr<XmsgNeUsr> nu, bool attached)
{
	unique_lock<mutex> lock(this->lock4negSub);
	auto it = this->negSub.find(nu->neg);
	if (it == this->negSub.end()) 
	{
		LOG_TRACE("no subscriber on network-element group: %s", nu->neg.c_str())
		return;
	}
	if (it->second->empty()) 
	{
		LOG_TRACE("no subscriber on network-element group: %s", nu->neg.c_str())
		return;
	}
	unordered_set<string> sub;
	for (auto& iter : *(it->second))
		sub.insert(iter);
	lock.unlock(); 
	for (auto& iter : sub) 
	{
		LOG_DEBUG("network-element channel lost, we will send notice to subscriber, ne: %s, subscriber: %s", nu->uid.c_str(), iter.c_str())
		auto xnu = XmsgNeGroupMgr::instance()->findByCgt(iter);
		xnu->future([nu, xnu, attached]
		{
			shared_ptr<XmsgMscPubNeGroupStatusNotice> notice(new XmsgMscPubNeGroupStatusNotice());
			notice->set_neg(nu->neg);
			notice->set_cgt(nu->uid);
			notice->set_attached(attached);
			notice->set_uts(DateMisc::nowGmt0());
			XmsgImChannel::cast(xnu->channel)->unidirection(notice);
		});
	}
}

XmsgMscMgr::~XmsgMscMgr()
{

}

