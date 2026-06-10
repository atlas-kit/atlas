// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_FILELOADER_H
#define FS_FILELOADER_H

namespace OTB {

using MappedFile = boost::iostreams::mapped_file_source;
using iterator = MappedFile::iterator;

struct Node
{
	std::vector<Node> children = {};
	iterator propsBegin, propsEnd;
	char type;

	static constexpr char ESCAPE = '\xFD';
	static constexpr char START = '\xFE';
	static constexpr char END = '\xFF';
};

class Loader
{
public:
	Loader(MappedFile file, Node root) : file{std::move(file)}, root{std::move(root)} {}

	Loader(Loader&&) = default;
	Loader& operator=(Loader&&) = default;

	// Delete copy operations to prevent accidental expensive copies
	Loader(const Loader&) = delete;
	Loader& operator=(const Loader&) = delete;

	const std::vector<Node>& children() const { return root.children; }
	auto begin() const { return root.propsBegin; }
	auto end() const { return root.propsEnd; }

private:
	MappedFile file;
	Node root;
};

Loader load(std::string_view filename, std::string_view acceptedIdentifier);

[[nodiscard]] std::string readBytes(OTB::iterator& first, const OTB::iterator last, const size_t len);
[[nodiscard]] std::string readString(OTB::iterator& first, const OTB::iterator last);
void skip(OTB::iterator& first, const OTB::iterator last, const size_t len);

template <class T>
[[nodiscard]] T read(OTB::iterator& first, const OTB::iterator last)
{
	static_assert(std::is_trivially_copyable_v<T>, "OTB::read requires a trivially copyable type");

	T out;
	char* dst = reinterpret_cast<char*>(&out);

	// Fast path: when none of the next sizeof(T) raw bytes is an escape marker we can copy the value
	// straight out of the memory-mapped file in one shot, avoiding the per-read std::string allocation
	// that readBytes() performs. Escape markers (0xFD) are rare in practice, so this is the common case
	// for the millions of small reads issued while loading a map. The bounds check below guarantees the
	// source range holds at least sizeof(T) bytes, and dst is exactly sizeof(T) bytes wide.
	if (last - first >= static_cast<std::ptrdiff_t>(sizeof(T))) {
		if (std::find(first, first + sizeof(T), Node::ESCAPE) == first + sizeof(T)) {
			std::copy_n(first, sizeof(T), dst);
			first += sizeof(T);
			return out;
		}
	}

	// Slow path: an escape marker is present (or we are near the end of the buffer). Unescape byte by
	// byte directly into the output, still without allocating an intermediate buffer.
	for (size_t i = 0; i < sizeof(T); ++i) {
		if (first == last) [[unlikely]] {
			throw std::invalid_argument("Not enough bytes to read.");
		}

		if (*first == Node::ESCAPE && ++first == last) [[unlikely]] {
			throw std::invalid_argument("Not enough bytes to read.");
		}

		dst[i] = *first++;
	}

	return out;
}

} // namespace OTB

class PropStream
{
public:
	void init(const char* a, size_t size)
	{
		p = a;
		end = a + size;
	}

	size_t size() const { return end - p; }

	template <typename T>
	bool read(T& ret)
	{
		if (size() < sizeof(T)) {
			return false;
		}

		memcpy(&ret, p, sizeof(T));
		p += sizeof(T);
		return true;
	}

	std::pair<std::string_view, bool> readString()
	{
		uint16_t strLen;
		if (!read<uint16_t>(strLen)) {
			return {"", false};
		}

		if (size() < strLen) {
			return {"", false};
		}

		std::string_view ret{p, strLen};
		p += strLen;
		return {ret, true};
	}

	bool skip(size_t n)
	{
		if (size() < n) {
			return false;
		}

		p += n;
		return true;
	}

private:
	const char* p = nullptr;
	const char* end = nullptr;
};

class PropWriteStream
{
public:
	PropWriteStream() = default;

	// non-copyable
	PropWriteStream(const PropWriteStream&) = delete;
	PropWriteStream& operator=(const PropWriteStream&) = delete;

	std::string_view getStream() const { return {buffer.data(), buffer.size()}; }

	void clear() { buffer.clear(); }

	template <typename T>
	void write(T add)
	{
		char* addr = reinterpret_cast<char*>(&add);
		std::copy(addr, addr + sizeof(T), std::back_inserter(buffer));
	}

	void writeString(std::string_view str)
	{
		size_t strLength = str.size();
		if (strLength > std::numeric_limits<uint16_t>::max()) {
			write<uint16_t>(0);
			return;
		}

		write(static_cast<uint16_t>(strLength));
		std::copy(str.begin(), str.end(), std::back_inserter(buffer));
	}

private:
	std::vector<char> buffer;
};

#endif // FS_FILELOADER_H
