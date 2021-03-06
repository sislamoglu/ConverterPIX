/*********************************************************************
*                   Copyright by mwl4 (C) 2015                      *
*********************************************************************
* File       : resource_lib.h
* Project    : ConverterPIX
* Developers : Michal Wojtowicz (mwl450@gmail.com)
			 : Piotr Krupa (piotrekwidzew1@gmail.com)
*********************************************************************/

#pragma once

#include <prerequisites.h>
#include <material/material.h>
#include <utils/explicit_singleton.h>

class ResourceLibrary : public ExplicitSingleton<ResourceLibrary>
{
public:
	using Entry = std::shared_ptr<TextureObject>;
private:
	std::unordered_map<std::string, Entry> m_tobjs;
public:
	Entry obtain(std::string basepath, std::string tobjfile);
	void destroy();
};

/* eof */
