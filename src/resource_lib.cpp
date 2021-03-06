/*********************************************************************
*                   Copyright by mwl4 (C) 2015                      *
*********************************************************************
* File       : resource_lib.cpp
* Project    : ConverterPIX
* Developers : Michal Wojtowicz (mwl450@gmail.com)
			 : Piotr Krupa (piotrekwidzew1@gmail.com)
*********************************************************************/

#include "resource_lib.h"

#include <texture/texture_object.h>

auto ResourceLibrary::obtain(std::string basepath, std::string tobjfile) -> Entry
{
	if (m_tobjs.find(tobjfile) == m_tobjs.end())
	{
		Entry texobj = std::make_shared<TextureObject>();
		if (texobj->load(basepath, tobjfile))
		{
			m_tobjs.insert({ tobjfile.c_str(), texobj });
		}
		else
		{
			printf("Unable to load: \"%s\"!\n", tobjfile.c_str());
			return nullptr;
		}
	}
	return m_tobjs[tobjfile];
}

void ResourceLibrary::destroy()
{
	m_tobjs.clear();
}

/* eof */
