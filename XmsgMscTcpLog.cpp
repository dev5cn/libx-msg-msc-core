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

#include "XmsgMscTcpLog.h"
#include "XmsgMscCfg.h"
#include "XmsgDst.h"

XmsgMscTcpLog::XmsgMscTcpLog() :
		XmsgImTcpLog()
{

}

void XmsgMscTcpLog::didaMainThread(ullong now)
{

}

void XmsgMscTcpLog::dida(ullong now)
{

}

void XmsgMscTcpLog::rx(XscChannel* channel, uchar* dat, int len)
{

}

void XmsgMscTcpLog::tx(XscChannel* channel, uchar* dat, int len)
{

}

void XmsgMscTcpLog::transInitStart(SptrXiti trans, shared_ptr<XscProtoPdu> pdu )
{
	if (trans->otrace == NULL)
		return;
	shared_ptr<XmsgDstEvent> evn(new XmsgDstEvent());
	evn->set_ne(XmsgMscCfg::instance()->cgt->toString());
	evn->set_tid(Net::hex2strLowerCase(trans->otrace->tid, sizeof(trans->otrace->tid)));
	evn->set_sid(Net::hex2strLowerCase(trans->otrace->sid, sizeof(trans->otrace->sid)));
	evn->set_pid(Net::hex2strLowerCase(trans->otrace->pid, sizeof(trans->otrace->pid)));
	evn->set_sne(trans->otrace->sne);
	evn->set_dne(trans->otrace->dne);
	evn->set_gts(trans->otrace->gts);
	int len;
	uchar* dat = pdu->bytes(&len);
	evn->set_pdu(dat, len);
	XmsgDst::instance()->write(evn);
}

void XmsgMscTcpLog::transInitFinished(SptrXiti trans, shared_ptr<XscProtoPdu> pdu )
{
	if (trans->itrace == NULL)
		return;
	shared_ptr<XmsgDstEvent> evn(new XmsgDstEvent());
	evn->set_ne(XmsgMscCfg::instance()->cgt->toString());
	evn->set_tid(Net::hex2strLowerCase(trans->itrace->tid, sizeof(trans->itrace->tid)));
	evn->set_sid(Net::hex2strLowerCase(trans->itrace->sid, sizeof(trans->itrace->sid)));
	evn->set_pid(Net::hex2strLowerCase(trans->itrace->pid, sizeof(trans->itrace->pid)));
	evn->set_sne(trans->itrace->sne);
	evn->set_dne(trans->itrace->dne);
	evn->set_gts(trans->itrace->gts);
	if (pdu != nullptr)
	{
		int len;
		uchar* dat = pdu->bytes(&len);
		evn->set_pdu(dat, len);
	}
	XmsgDst::instance()->write(evn);
}

void XmsgMscTcpLog::transPassStart(SptrXitp trans, shared_ptr<XscProtoPdu> pdu )
{
	if (trans->itrace == NULL)
		return;
	shared_ptr<XmsgDstEvent> evn(new XmsgDstEvent());
	evn->set_ne(XmsgMscCfg::instance()->cgt->toString());
	evn->set_tid(Net::hex2strLowerCase(trans->itrace->tid, sizeof(trans->itrace->tid)));
	evn->set_sid(Net::hex2strLowerCase(trans->itrace->sid, sizeof(trans->itrace->sid)));
	evn->set_pid(Net::hex2strLowerCase(trans->itrace->pid, sizeof(trans->itrace->pid)));
	evn->set_sne(trans->itrace->sne);
	evn->set_dne(trans->itrace->dne);
	evn->set_gts(trans->itrace->gts);
	int len;
	uchar* dat = pdu->bytes(&len);
	evn->set_pdu(dat, len);
	XmsgDst::instance()->write(evn);
}

void XmsgMscTcpLog::transPassFinished(SptrXitp trans, shared_ptr<XscProtoPdu> pdu )
{
	if (trans->otrace == NULL)
		return;
	shared_ptr<XmsgDstEvent> evn(new XmsgDstEvent());
	evn->set_ne(XmsgMscCfg::instance()->cgt->toString());
	evn->set_tid(Net::hex2strLowerCase(trans->otrace->tid, sizeof(trans->otrace->tid)));
	evn->set_sid(Net::hex2strLowerCase(trans->otrace->sid, sizeof(trans->otrace->sid)));
	evn->set_pid(Net::hex2strLowerCase(trans->otrace->pid, sizeof(trans->otrace->pid)));
	evn->set_sne(trans->otrace->sne);
	evn->set_dne(trans->otrace->dne);
	evn->set_gts(trans->otrace->gts);
	if (pdu != nullptr)
	{
		int len;
		uchar* dat = pdu->bytes(&len);
		evn->set_pdu(dat, len);
	}
	XmsgDst::instance()->write(evn);
}

void XmsgMscTcpLog::transInitUni(SptrXitui trans, shared_ptr<XscProtoPdu> pdu )
{
	if (trans->otrace == NULL)
		return;
	shared_ptr<XmsgDstEvent> evn(new XmsgDstEvent());
	evn->set_ne(XmsgMscCfg::instance()->cgt->toString());
	evn->set_tid(Net::hex2strLowerCase(trans->otrace->tid, sizeof(trans->otrace->tid)));
	evn->set_sid(Net::hex2strLowerCase(trans->otrace->sid, sizeof(trans->otrace->sid)));
	evn->set_pid(Net::hex2strLowerCase(trans->otrace->pid, sizeof(trans->otrace->pid)));
	evn->set_sne(trans->otrace->sne);
	evn->set_dne(trans->otrace->dne);
	evn->set_gts(trans->otrace->gts);
	int len;
	uchar* dat = pdu->bytes(&len);
	evn->set_pdu(dat, len);
	XmsgDst::instance()->write(evn);
}

void XmsgMscTcpLog::transPassUni(SptrXitup trans, shared_ptr<XscProtoPdu> pdu )
{
	if (trans->itrace == NULL)
		return;
	shared_ptr<XmsgDstEvent> evn(new XmsgDstEvent());
	evn->set_ne(XmsgMscCfg::instance()->cgt->toString());
	evn->set_tid(Net::hex2strLowerCase(trans->itrace->tid, sizeof(trans->itrace->tid)));
	evn->set_sid(Net::hex2strLowerCase(trans->itrace->sid, sizeof(trans->itrace->sid)));
	evn->set_pid(Net::hex2strLowerCase(trans->itrace->pid, sizeof(trans->itrace->pid)));
	evn->set_sne(trans->itrace->sne);
	evn->set_dne(trans->itrace->dne);
	evn->set_gts(trans->itrace->gts);
	int len;
	uchar* dat = pdu->bytes(&len);
	evn->set_pdu(dat, len);
	XmsgDst::instance()->write(evn);
}

XmsgMscTcpLog::~XmsgMscTcpLog()
{

}
