/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          http://www.mrpt.org/                          |
   |                                                                        |
   | Copyright (c) 2005-2017, Individual contributors, see AUTHORS file     |
   | See: http://www.mrpt.org/Authors - All rights reserved.                |
   | Released under BSD License. See details in http://www.mrpt.org/License |
   +------------------------------------------------------------------------+ */

#include "io-precomp.h"  // Precompiled headers

#include <mrpt/io/CFileGZInputStream.h>
#include <mrpt/system/filesystem.h>
#include <mrpt/core/exceptions.h>

#include <zlib.h>

using namespace mrpt::io;
using namespace std;

static_assert(
	!std::is_copy_constructible<CFileGZInputStream>::value &&
	!std::is_copy_assignable<CFileGZInputStream>::value,
	"Copy Check");

#define THE_GZFILE reinterpret_cast<gzFile>(m_f)

CFileGZInputStream::CFileGZInputStream(const string& fileName) : m_f(nullptr)
{
	MRPT_START
	open(fileName);
	MRPT_END
}

CFileGZInputStream::CFileGZInputStream() : m_f(nullptr) {}
bool CFileGZInputStream::open(const std::string& fileName)
{
	MRPT_START

	if (m_f) gzclose(THE_GZFILE);

	// Get compressed file size:
	m_file_size = mrpt::system::getFileSize(fileName);
	if (m_file_size == uint64_t(-1))
		THROW_EXCEPTION_FMT("Couldn't access the file '%s'", fileName.c_str());

	// Open gz stream:
	m_f = gzopen(fileName.c_str(), "rb");
	return m_f != nullptr;

	MRPT_END
}

void CFileGZInputStream::close()
{
	if (m_f)
	{
		gzclose(THE_GZFILE);
		m_f = nullptr;
	}
}

CFileGZInputStream::~CFileGZInputStream() { close(); }
size_t CFileGZInputStream::Read(void* Buffer, size_t Count)
{
	if (!m_f)
	{
		THROW_EXCEPTION("File is not open.");
	}

	return gzread(THE_GZFILE, Buffer, Count);
}

size_t CFileGZInputStream::Write(const void* Buffer, size_t Count)
{
	MRPT_UNUSED_PARAM(Buffer);
	MRPT_UNUSED_PARAM(Count);
	THROW_EXCEPTION("Trying to write to an input file stream.");
}

uint64_t CFileGZInputStream::getTotalBytesCount() const
{
	if (!m_f)
	{
		THROW_EXCEPTION("File is not open.");
	}
	return m_file_size;
}

uint64_t CFileGZInputStream::getPosition() const
{
	if (!m_f)
	{
		THROW_EXCEPTION("File is not open.");
	}
	return gztell(THE_GZFILE);
}

bool CFileGZInputStream::fileOpenCorrectly() const { return m_f != nullptr; }
bool CFileGZInputStream::checkEOF()
{
	if (!m_f)
		return true;
	else
		return 0 != gzeof(THE_GZFILE);
}

uint64_t CFileGZInputStream::Seek(int64_t, CStream::TSeekOrigin)
{
	THROW_EXCEPTION("Method not available in this class.");
}
