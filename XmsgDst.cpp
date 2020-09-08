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

#include "XmsgDst.h"
#include "XmsgMscCfg.h"

XmsgDst* XmsgDst::inst = new XmsgDst();

XmsgDst::XmsgDst()
{

}

XmsgDst* XmsgDst::instance()
{
	return XmsgDst::inst;
}

bool XmsgDst::init()
{
	return this->startKafkaProd();
}

void XmsgDst::write(SptrCgt cgt, shared_ptr<XscProtoPdu> pdu)
{
	shared_ptr<XmsgDstEvent> evn(new XmsgDstEvent());
	evn->set_ne(cgt->toString());
	evn->set_tid(Net::hex2strLowerCase(pdu->transm.header->trace->tid, sizeof(pdu->transm.header->trace->tid)));
	evn->set_sid(Net::hex2strLowerCase(pdu->transm.header->trace->sid, sizeof(pdu->transm.header->trace->sid)));
	evn->set_pid(Net::hex2strLowerCase(pdu->transm.header->trace->pid, sizeof(pdu->transm.header->trace->pid)));
	evn->set_sne(pdu->transm.header->trace->sne);
	evn->set_dne(pdu->transm.header->trace->dne);
	evn->set_gts(pdu->transm.header->trace->gts);
	int len;
	uchar* dat = pdu->bytes(&len);
	evn->set_pdu(dat, len);
	XmsgDst::instance()->write(evn);
}

void XmsgDst::write(shared_ptr<XmsgDstEvent> evn)
{
	this->kafkaProd->send(evn);
}

bool XmsgDst::startKafkaProd()
{
	this->kafkaProd.reset(new KafkaMqProd());
	map<string, string> cfg;
	for (auto& it : XmsgMscCfg::instance()->cfgPb->kafkaprod())
	{
		if ("bootstrap.servers" == it.first)
			continue;
		if ("topic" == it.first)
			continue;
		cfg.insert(make_pair<>(it.first, it.second));
	}
	bool ret = this->kafkaProd->init(XmsgMisc::getStr(XmsgMscCfg::instance()->cfgPb->kafkaprod(), "bootstrap.servers"), 
	XmsgMisc::getStr(XmsgMscCfg::instance()->cfgPb->kafkaprod(), "topic"), cfg);
	if (!ret)
	{
		LOG_ERROR("start kafka producer failed, please check network env or settings")
		return false;
	}
	return true;
}

XmsgDst::~XmsgDst()
{

}
