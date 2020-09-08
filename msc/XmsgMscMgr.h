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

#ifndef MSC_XMSGMSCMGR_H_
#define MSC_XMSGMSCMGR_H_

#include "XmsgMscSubordinate.h"
#include "XmsgMscSuperior.h"

class XmsgMscMgr
{
public:
	void addSuperior(shared_ptr<XmsgMscSuperior> superior); 
	shared_ptr<XmsgMscSuperior> chooseSuperior(SptrCgt cgt); 
	void initSubordinate(const string& node, shared_ptr<vector<string>> cgt); 
	shared_ptr<XmsgMscSubordinate> addSubordinate(const string& node, shared_ptr<XmsgMscSubordinate> subordinate); 
	shared_ptr<XmsgMscSubordinate> resetSubordinate(const string& node, SptrCgt cgt); 
	shared_ptr<XmsgMscSubordinate> chooseSubordinate(const string& node, SptrCgt cgt); 
	bool isSuperior(SptrCgt cgt); 
	bool isSubordinate(SptrCgt cgt, string& node); 
	bool isRoute2superior(SptrCgt dest); 
	bool isRoute2subordinate(SptrCgt dest); 
	static XmsgMscMgr* instance();
public:
	static void route2service(XscChannel* channel, shared_ptr<XscProtoPdu> pdu, SptrCgt dest); 
	static void route2subordinate(XscChannel* channel, shared_ptr<XscProtoPdu> pdu, SptrCgt dest); 
	static void route2superior(XscChannel* channel, shared_ptr<XscProtoPdu> pdu, SptrCgt dest); 
public:
	void subNeGroup(const string& cgt , const string& neg ); 
	void removeSub(const string& cgt ); 
	static void xmsgNeUsrEvnEstab(shared_ptr<XmsgNeUsr> nu); 
	static void xmsgNeUsrEvnDisc(shared_ptr<XmsgNeUsr> nu); 
private:
	vector<shared_ptr<XmsgMscSuperior>> superior; 
	unordered_map<string , shared_ptr<vector<pair<string , shared_ptr<XmsgMscSubordinate>>>>> subordinate; 
	mutex lock4subordinate; 
	static void forward(shared_ptr<XscUsr> usr, XscChannel* channel, shared_ptr<XscProtoPdu> pdu); 
	static void routeFailed(XscChannel* channel, shared_ptr<XscProtoPdu> pdu, ushort ret, const string& desc); 
private:
	unordered_map<string , shared_ptr<unordered_set<string>> > subNeg; 
	unordered_map<string , shared_ptr<unordered_set<string>> > negSub; 
	mutex lock4negSub;
private:
	static XmsgMscMgr* inst;
	void pubXmsgNeUsrEvn(shared_ptr<XmsgNeUsr> nu, bool attached); 
	XmsgMscMgr();
	virtual ~XmsgMscMgr();
};

#endif 
