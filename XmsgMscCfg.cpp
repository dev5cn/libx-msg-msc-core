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

#include <libx-msg-common-dat-struct-cpp.h>
#include "XmsgMscCfg.h"

shared_ptr<XmsgMscCfg> XmsgMscCfg::cfg; 

XmsgMscCfg::XmsgMscCfg()
{

}

shared_ptr<XmsgMscCfg> XmsgMscCfg::instance()
{
	return XmsgMscCfg::cfg;
}

shared_ptr<XmsgMscCfg> XmsgMscCfg::load(const char* path)
{
	XMLDocument doc;
	if (doc.LoadFile(path) != 0)
	{
		LOG_ERROR("load config file failed, path: %s", path)
		return nullptr;
	}
	XMLElement* root = doc.RootElement();
	if (root == NULL)
	{
		LOG_ERROR("it a empty xml file? path: %s", path)
		return nullptr;
	}
	shared_ptr<XmsgMscCfg> cfg(new XmsgMscCfg());
	cfg->cfgPb.reset(new XmsgMscCfgPb());
	cfg->cgt = ChannelGlobalTitle::parse(Misc::strAtt(root, "cgt"));
	if (cfg->cgt == nullptr)
	{
		LOG_ERROR("load config file failed, cgt format error: %s", Misc::strAtt(root, "cgt").c_str())
		return nullptr;
	}
	cfg->cfgPb->set_cgt(cfg->cgt->toString());
	if (!cfg->loadLogCfg(root))
		return nullptr;
	if (!cfg->loadXscServerCfg(root))
		return nullptr;
	if (!cfg->loadXmsgNeN2hCfg(root))
		return nullptr;
	if (!cfg->loadXmsgMscSuperiorCfg(root))
		return nullptr;
	if (!cfg->loadXmsgMscSubordinateCfg(root))
		return nullptr;
	if (!cfg->loadKafkaProdCfg(root))
		return nullptr;
	if (!cfg->loadMiscCfg(root))
		return nullptr;
	if (cfg->cfgPb->superior().empty() && cfg->cfgPb->subordinate().empty() && cfg->cfgPb->n2h().empty())
	{
		LOG_FAULT("superior, subordinate, service all empty, what do you want?")
		return nullptr;
	}
	LOG_INFO("load config file successful, cfg: %s", cfg->toString().c_str())
	XmsgMscCfg::setCfg(cfg);
	return cfg;
}

void XmsgMscCfg::setCfg(shared_ptr<XmsgMscCfg> cfg)
{
	XmsgMscCfg::cfg = cfg;
}

shared_ptr<XscTcpCfg> XmsgMscCfg::pubXscServerCfg()
{
	return this->xmsgMscCfgXscTcpServer2xscTcpCfg(&this->cfgPb->pub());
}

shared_ptr<XscTcpCfg> XmsgMscCfg::priXscServerCfg()
{
	return this->xmsgMscCfgXscTcpServer2xscTcpCfg(&this->cfgPb->pri());
}

shared_ptr<XscTcpCfg> XmsgMscCfg::xmsgMscCfgXscTcpServer2xscTcpCfg(const XmsgMscCfgXscTcpServer* pb)
{
	shared_ptr<XscTcpCfg> tcpCfg(new XscTcpCfg());
	tcpCfg->addr = pb->addr();
	tcpCfg->worker = pb->worker();
	tcpCfg->peerLimit = pb->peerlimit();
	tcpCfg->peerMtu = pb->peermtu();
	tcpCfg->peerRcvBuf = pb->peerrcvbuf();
	tcpCfg->peerSndBuf = pb->peersndbuf();
	tcpCfg->lazyClose = pb->lazyclose();
	tcpCfg->tracing = pb->tracing();
	tcpCfg->heartbeat = pb->heartbeat();
	tcpCfg->n2hZombie = pb->n2hzombie();
	tcpCfg->n2hTransTimeout = pb->n2htranstimeout();
	tcpCfg->n2hTracing = pb->n2htracing();
	tcpCfg->h2nReConn = pb->h2nreconn();
	tcpCfg->h2nTransTimeout = pb->h2ntranstimeout();
	return tcpCfg;
}

bool XmsgMscCfg::loadXscServerCfg(XMLElement* root)
{
	auto node = root->FirstChildElement("xsc-server"); 
	while (node != NULL)
	{
		string name;
		Misc::strAtt(node, "name", &name);
		if ("pub-tcp" == name)
		{
			auto pub = this->loadXscTcpCfg(node);
			if (pub == nullptr)
				return false;
			this->cfgPb->mutable_pub()->CopyFrom(*pub);
		} else if ("pri-tcp" == name)
		{
			auto pri = this->loadXscTcpCfg(node);
			if (pri == nullptr)
				return false;
			this->cfgPb->mutable_pri()->CopyFrom(*pri);
		}
		node = node->NextSiblingElement("xsc-server");
	}
	if (!this->cfgPb->has_pub() || !this->cfgPb->has_pri())
	{
		LOG_ERROR("load x-msg-im-ap config failed, node: <xsc-server>")
		return false;
	}
	return true;
}

bool XmsgMscCfg::loadLogCfg(XMLElement* root)
{
	auto node = root->FirstChildElement("log");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <log>")
		return false;
	}
	XmsgMscCfgLog* log = this->cfgPb->mutable_log();
	log->set_level(Misc::toUpercase(Misc::strAtt(node, "level")));
	log->set_output(Misc::toUpercase(Misc::strAtt(node, "output")));
	return true;
}

shared_ptr<XmsgMscCfgXscTcpServer> XmsgMscCfg::loadXscTcpCfg(XMLElement* node)
{
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <xsc-server>")
		return nullptr;
	}
	shared_ptr<XmsgMscCfgXscTcpServer> tcpServer(new XmsgMscCfgXscTcpServer());
	tcpServer->set_addr(Misc::strAtt(node, "addr"));
	tcpServer->set_worker(Misc::hexOrInt(node, "worker"));
	tcpServer->set_peerlimit(Misc::hexOrInt(node, "peerLimit"));
	tcpServer->set_peermtu(Misc::hexOrInt(node, "peerMtu"));
	tcpServer->set_peerrcvbuf(Misc::hexOrInt(node, "peerRcvBuf"));
	tcpServer->set_peersndbuf(Misc::hexOrInt(node, "peerSndBuf"));
	tcpServer->set_lazyclose(Misc::hexOrInt(node, "lazyClose"));
	tcpServer->set_tracing("true" == Misc::strAtt(node, "tracing"));
	tcpServer->set_heartbeat(Misc::hexOrInt(node, "heartbeat"));
	tcpServer->set_n2hzombie(Misc::hexOrInt(node, "n2hZombie"));
	tcpServer->set_n2htranstimeout(Misc::hexOrInt(node, "n2hTransTimeout"));
	tcpServer->set_n2htracing("true" == Misc::strAtt(node, "n2hTracing"));
	tcpServer->set_h2nreconn(Misc::hexOrInt(node, "h2nReConn"));
	tcpServer->set_h2ntranstimeout(Misc::hexOrInt(node, "h2nTransTimeout"));
	return tcpServer;
}

bool XmsgMscCfg::loadXmsgMscSuperiorCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("x-msg-msc-superior");
	if (node == NULL) 
		return true;
	string domainHlr = "";
	node = node->FirstChildElement("msc");
	while (node != NULL)
	{
		auto superior = this->cfgPb->add_superior();
		Misc::strAtt(node, "cgt", superior->mutable_cgt());
		Misc::strAtt(node, "addr", superior->mutable_addr());
		Misc::strAtt(node, "pwd", superior->mutable_pwd());
		Misc::strAtt(node, "alg", superior->mutable_alg());
		SptrCgt cgt = ChannelGlobalTitle::parse(superior->cgt());
		if (superior->addr().empty() || superior->pwd().empty() || cgt == nullptr)
		{
			LOG_ERROR("load config failed, node: <x-msg-msc-superior><msc>, msc: %s", superior->ShortDebugString().c_str())
			return false;
		}
		if (domainHlr == "")
			domainHlr = cgt->domain + "." + cgt->hlr;
		if (domainHlr != "" && domainHlr != cgt->domain + "." + cgt->hlr) 
		{
			LOG_ERROR("load config failed, x-msg-msc channel global title domain-hlr must be same: %s", superior->ShortDebugString().c_str())
			return false;
		}
		node = node->NextSiblingElement("msc");
	}
	return true;
}

bool XmsgMscCfg::loadXmsgMscSubordinateCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("x-msg-msc-subordinate");
	if (node == NULL) 
		return true;
	node = node->FirstChildElement("msc");
	unordered_map<string , string > domainHlr;
	while (node != NULL)
	{
		auto subordinate = this->cfgPb->add_subordinate();
		Misc::strAtt(node, "cgt", subordinate->mutable_cgt());
		Misc::strAtt(node, "pwd", subordinate->mutable_pwd());
		Misc::strAtt(node, "addr", subordinate->mutable_addr());
		SptrCgt cgt = ChannelGlobalTitle::parse(subordinate->cgt());
		if (subordinate->pwd().empty() || cgt == nullptr)
		{
			LOG_ERROR("load config failed, node: <x-msg-msc-subordinate><msc>, msc: %s", subordinate->ShortDebugString().c_str())
			return false;
		}
		auto it = domainHlr.find(cgt->domain);
		if (it == domainHlr.end())
			domainHlr[cgt->domain] = cgt->hlr;
		else
		{
			if (it->second != cgt->hlr) 
			{
				LOG_ERROR("load config failed, x-msg-msc channel global title domain-hlr must be same: %s", subordinate->ShortDebugString().c_str())
				return false;
			}
		}
		node = node->NextSiblingElement("msc");
	}
	return true;
}

bool XmsgMscCfg::loadXmsgNeN2hCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("ne-group-n2h");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: ne-group-n2h")
		return false;
	}
	node = node->FirstChildElement("ne");
	while (node != NULL)
	{
		auto ne = this->cfgPb->add_n2h();
		Misc::strAtt(node, "neg", ne->mutable_neg());
		Misc::strAtt(node, "cgt", ne->mutable_cgt());
		Misc::strAtt(node, "pwd", ne->mutable_pwd());
		Misc::strAtt(node, "addr", ne->mutable_addr());
		SptrCgt cgt = ChannelGlobalTitle::parse(ne->cgt());
		if (ne->neg().empty() || ne->pwd().empty() || cgt == nullptr)
		{
			LOG_ERROR("load config failed, node: <ne-group-n2h><ne>, ne: %s", ne->ShortDebugString().c_str())
			return false;
		}
		node = node->NextSiblingElement("ne");
	}
	return true;
}

bool XmsgMscCfg::loadKafkaProdCfg(XMLElement* root)
{
	auto node = root->FirstChildElement("kafka-prod");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <kafka-prod>")
		return false;
	}
	auto prod = this->cfgPb->mutable_kafkaprod();
	node = node->FirstChildElement("kv");
	while (node != NULL)
	{
		string key = Misc::strAtt(node, "key");
		string val = Misc::strAtt(node, "val");
		if (key.empty() || val.empty())
		{
			LOG_ERROR("load config failed, node: <kafka-prod>")
			return false;
		}
		XmsgMisc::insertKv(prod, key, val);
		node = node->NextSiblingElement("kv");
	}
	if (prod->find("bootstrap.servers") == prod->end())
	{
		LOG_ERROR("load config failed, node: <kafka-prod>, missing required parameter: bootstrap.servers")
		return false;
	}
	if (prod->find("topic") == prod->end())
	{
		LOG_ERROR("load config failed, node: <kafka-prod>, missing required parameter: topic")
		return false;
	}
	return true;
}

bool XmsgMscCfg::loadMiscCfg(XMLElement* root)
{
	auto node = root->FirstChildElement("misc");
	if (node == NULL)
	{
		LOG_ERROR("load  config failed, node: <misc>")
		return false;
	}
	return true;
}

string XmsgMscCfg::toString()
{
	return this->cfgPb->ShortDebugString();
}

XmsgMscCfg::~XmsgMscCfg()
{

}

