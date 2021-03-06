/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : animation.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include "animation.h"
#include <file.h>
#include <structs/pma.h>
#include <model/model.h>

#include <glm/gtx/transform.hpp>

using namespace prism;

bool Animation::load(std::shared_ptr<Model> model, std::string filePath)
{
	if (!model->loaded())
	{
		printf("[anim] Model is not loaded!");
		return false;
	}

	m_model = model;
	if (filePath[0] == '/')
	{
		m_filePath = filePath;
	}
	else
	{
		m_filePath = m_model->fileDirectory() + "/" + filePath;
	}

	std::string pmaFilepath = model->basePath() + m_filePath + ".pma";

	File file;
	if (!file.open(pmaFilepath, "rb"))
	{
		printf("[anim] Cannot open animation file: \"%s\"! %s\n", pmaFilepath.c_str(), strerror(errno));
		return false;
	}

	size_t fileSize = file.getSize();
	std::unique_ptr<uint8_t[]> buffer(new uint8_t[fileSize]);
	file.read((char *)buffer.get(), sizeof(uint8_t), fileSize);
	file.close();

	pma_header *header = (pma_header *)(buffer.get());
	if (header->m_version != pma_header::SUPPORTED_VERSION)
	{
		printf("[anim] Invalid version of animation file! (have: %i, expected: %i\n", header->m_version, pma_header::SUPPORTED_VERSION);
		return false;
	}

	m_name = token_to_string(header->m_name);
	m_totalLength = header->m_anim_length;

	m_bones.resize(header->m_bones);
	m_frames.resize(header->m_bones);
	m_timeframes.resize(header->m_frames);

	for (uint32_t i = 0; i < m_bones.size(); ++i)
	{
		m_bones[i] = *(uint8_t *)(buffer.get() + header->m_bones_offset + i*sizeof(uint8_t));
		m_frames[i].resize(header->m_frames);
		for (uint32_t j = 0; j < header->m_frames; ++j)
		{
			pma_frame *frame = (pma_frame *)(buffer.get() + header->m_frames_offset + i*sizeof(pma_frame) + j*m_bones.size()*sizeof(pma_frame));
			if(i == 0) m_timeframes[j] = *(float *)(buffer.get() + header->m_lengths_offset + j * sizeof(float));
			m_frames[i][j].m_stretch = frame->m_stretch;
			m_frames[i][j].m_rotation = frame->m_rot;
			m_frames[i][j].m_translation = frame->m_trans;
			m_frames[i][j].m_scale = frame->m_scale;
		}
	}

	if (header->m_flags == 2)
	{
		m_movement = std::make_unique<std::vector<Float3>>(header->m_frames);
		for (u32 i = 0; i < header->m_frames; ++i)
		{
			(*m_movement)[i] = *((float3 *)(buffer.get() + header->m_delta_trans_offset) + i);
		}
	}

	return true;
}

void Animation::saveToPia(std::string exportPath) const
{
	const std::string filename = m_filePath.substr(m_filePath.rfind('/') + 1);
	const std::string piafile = exportPath + m_filePath + ".pia";
	File file;
	if (!file.open(piafile.c_str(), "wb"))
	{
		printf("Cannot open file: \"%s\"! %s\n", piafile.c_str(), strerror(errno));
		return;
	}

	file << fmt::sprintf(
		"Header {"						SEOL
		TAB "FormatVersion: 3"			SEOL
		TAB "Source: \"%s\""			SEOL
		TAB "Type: \"Animation\""		SEOL
		TAB "Name: \"%s\""				SEOL
		"}"								SEOL,
			STRING_VERSION,
			filename.c_str()
		);

	file << fmt::sprintf(
		"Global {"						SEOL
		TAB "Skeleton: \"%s\""			SEOL
		TAB "TotalTime: %f"				SEOL
		TAB "BoneChannelCount: %i"		SEOL
		TAB "CustomChannelCount: %i"	SEOL
		"}"								SEOL,
			relativePath((m_model->filePath() + ".pis"), directory(m_filePath)).c_str(),
			m_totalLength,
			(int)m_bones.size(),
			0
		);

	if (m_movement)
	{
		file << "CustomChannel {"			SEOL;
		file << fmt::sprintf(
			TAB "Name: \"%s\""				SEOL
			TAB "StreamCount: %i"			SEOL
			TAB "KeyframeCount : %i"		SEOL,
				"Prism Movement",
				2,
				(int)m_timeframes.size()
			);
		file << TAB "Stream {"		SEOL;
		{
			file << fmt::sprintf(
				TAB TAB "Format: FLOAT"		SEOL
				TAB TAB "Tag: \"_TIME\""	SEOL
				);

			for (uint32_t j = 0; j < m_timeframes.size(); ++j)
			{
				file << fmt::sprintf(
					TAB TAB "%-5i( " FLT_FT " )" SEOL,
						j, flh(m_timeframes[j])
					);
			}
		}
		file << TAB "}"				SEOL;
		file << TAB "Stream {"				SEOL;
		{
			file << fmt::sprintf(
				TAB TAB "Format: FLOAT3"	SEOL
				TAB TAB "Tag: \"_MOVEMENT\"" SEOL
				);

			for (uint32_t j = 0; j < m_timeframes.size(); ++j)
			{
				file << fmt::sprintf(
					TAB TAB "%-5i( " FLT_FT "  " FLT_FT "  " FLT_FT " )" SEOL,
						j, flh((*m_movement)[j][0]), flh((*m_movement)[j][1]), flh((*m_movement)[j][2])
					);
			}
		}
		file << TAB "}"						SEOL;
		file << "}"							SEOL;
	}

	for (uint32_t i = 0; i < m_bones.size(); ++i)
	{
		if (m_bones[i] >= m_model->boneCount())
		{
			printf("[anim] %s: Bone index outside bones array! [%i/%i]\n", filename.c_str(), (int)m_bones[i], m_model->boneCount());
			return;
		}

		file << "BoneChannel {"			SEOL;
		{
			file << fmt::sprintf(
				TAB "Name: \"%s\""		SEOL
				TAB "StreamCount: %i"	SEOL
				TAB "KeyframeCount: %i"	SEOL,
					m_model->bone(m_bones[i])->m_name.c_str(),
					2,
					m_timeframes.size()
				);
			file << TAB "Stream {"		SEOL;
			{
				file << fmt::sprintf(
					TAB TAB "Format: FLOAT"		SEOL
					TAB TAB "Tag: \"_TIME\""	SEOL
					);

				for (uint32_t j = 0; j < m_timeframes.size(); ++j)
				{
					file << fmt::sprintf(
						TAB TAB "%-5i( " FLT_FT " )" SEOL,
							j, flh(m_timeframes[j])
						);
				}
			}
			file << TAB "}"				SEOL;

			file << TAB "Stream {"		SEOL;
			{
				file << fmt::sprintf(
					TAB TAB "Format: FLOAT4x4"	SEOL
					TAB TAB "Tag: \"_MATRIX\""	SEOL
					);

				for (uint32_t j = 0; j < m_timeframes.size(); ++j)
				{
					const Frame *frame = &m_frames[i][j];
					const glm::vec3 trans = glm_cast(frame->m_translation);
					const glm::quat rot = glm_cast(frame->m_rotation);
					const glm::vec3 scale = glm_cast(frame->m_scale);
					const prism::mat4 mat = glm::translate(trans) * glm::rotate(glm::angle(rot), glm::axis(rot)) * glm::scale(scale);

					file << fmt::sprintf(
						TAB TAB  "%-5i(  &%08x  &%08x  &%08x  &%08x"	SEOL
						TAB TAB "        &%08x  &%08x  &%08x  &%08x"	SEOL
						TAB TAB "        &%08x  &%08x  &%08x  &%08x"	SEOL
						TAB TAB "        &%08x  &%08x  &%08x  &%08x )"	SEOL,
							j,	flh(mat[0][0]), flh(mat[0][1]), flh(mat[0][2]), flh(mat[0][3]),
								flh(mat[1][0]), flh(mat[1][1]), flh(mat[1][2]), flh(mat[1][3]),
								flh(mat[2][0]), flh(mat[2][1]), flh(mat[2][2]), flh(mat[2][3]),
								flh(mat[3][0]), flh(mat[3][1]), flh(mat[3][2]), flh(mat[3][3])
						);
				}
			}
			file << TAB "}"				SEOL;
		}
		file << "}"						SEOL;
	}

	file.close();
}

/* eof */
