#include "memory_buffer.h"

EQEmu::MemoryBuffer::MemoryBuffer() {
	buffer_ = nullptr;
	size_ = 0;
	capacity_ = 0;
	read_pos_ = 0;
	write_pos_ = 0;
}

EQEmu::MemoryBuffer::MemoryBuffer(size_t sz) {
	buffer_ = nullptr;
	size_ = 0;
	capacity_ = 0;
	read_pos_ = 0;
	write_pos_ = 0;
	Resize(sz);
}

EQEmu::MemoryBuffer::MemoryBuffer(const MemoryBuffer &other) {
	if(other.capacity_) {
		buffer_ = new uchar[other.capacity_];
		memcpy(buffer_, other.buffer_, other.capacity_);
	} else {
		buffer_ = nullptr;
	}

	size_ = other.size_;
	capacity_ = other.capacity_;
	write_pos_ = other.write_pos_;
	read_pos_ = other.read_pos_;
}

EQEmu::MemoryBuffer::MemoryBuffer(MemoryBuffer &&other) {
	buffer_ = other.buffer_;
	size_ = other.size_;
	capacity_ = other.capacity_;
	write_pos_ = other.write_pos_;
	read_pos_ = other.read_pos_;

	other.buffer_ = nullptr;
	other.size_ = 0;
	other.capacity_ = 0;
	other.read_pos_ = 0;
	other.write_pos_ = 0;
}

EQEmu::MemoryBuffer& EQEmu::MemoryBuffer::operator=(const MemoryBuffer &other) {
	if(other.capacity_) {
		buffer_ = new uchar[other.capacity_];
		memcpy(buffer_, other.buffer_, other.capacity_);
	}
	else {
		buffer_ = nullptr;
	}

	size_ = other.size_;
	capacity_ = other.capacity_;
	write_pos_ = other.write_pos_;
	read_pos_ = other.read_pos_;
	return *this;
}

EQEmu::MemoryBuffer& EQEmu::MemoryBuffer::operator=(MemoryBuffer &&other) {
	buffer_ = other.buffer_;
	size_ = other.size_;
	capacity_ = other.capacity_;
	write_pos_ = other.write_pos_;
	read_pos_ = other.read_pos_;

	other.buffer_ = nullptr;
	other.size_ = 0;
	other.capacity_ = 0;
	other.read_pos_ = 0;
	other.write_pos_ = 0;
	return *this;
}

EQEmu::MemoryBuffer& EQEmu::MemoryBuffer::operator+=(const MemoryBuffer &rhs) {
	if(!rhs.buffer_) {
		return *this;
	}

	if(buffer_) {
		size_t old_size = size_;
		Resize(size_ + rhs.size_);
		memcpy(&buffer_[old_size], rhs.buffer_, rhs.size_);
	} else {
		buffer_ = new uchar[rhs.capacity_];
		memcpy(buffer_, rhs.buffer_, rhs.capacity_);
		size_ = rhs.size_;
		capacity_ = rhs.capacity_;
	}

	return *this;
}

EQEmu::MemoryBuffer::~MemoryBuffer() { Clear(); }

uchar& EQEmu::MemoryBuffer::operator[](size_t pos) {
	return buffer_[pos];
}

const uchar& EQEmu::MemoryBuffer::operator[](size_t pos) const {
	return buffer_[pos];
}

bool EQEmu::MemoryBuffer::Empty() {
	return size_ == 0;
}

bool EQEmu::MemoryBuffer::Empty() const {
	return size_ == 0;
}

size_t EQEmu::MemoryBuffer::Size() {
	return size_;
}

size_t EQEmu::MemoryBuffer::Size() const {
	return size_;
}

size_t EQEmu::MemoryBuffer::Capacity() {
	return capacity_;
}

size_t EQEmu::MemoryBuffer::Capacity() const {
	return capacity_;
}

void EQEmu::MemoryBuffer::Resize(size_t sz) {
	if(!buffer_) {
		size_t new_size = sz + 64;
		buffer_ = new uchar[new_size];
		capacity_ = new_size;
		size_ = sz;
		memset(buffer_, 0, capacity_);
		return;
	}

	if(sz > capacity_) {
		size_t new_size = sz + 32;
		uchar *temp = new uchar[new_size];
		memcpy(temp, buffer_, new_size);
		delete[] buffer_;
		buffer_ = temp;
		
		capacity_ = new_size;
		size_ = sz;
	}
	else {
		size_ = sz;
	}
}

void EQEmu::MemoryBuffer::Clear() {
	if(buffer_) {
		delete[] buffer_;
		buffer_ = nullptr;
	}
	
	size_ = 0;
	capacity_ = 0;
	write_pos_ = 0;
	read_pos_ = 0;
}

void EQEmu::MemoryBuffer::Zero() {
	if(buffer_) {
		memset(buffer_, 0, capacity_);
	}
}

void EQEmu::MemoryBuffer::Write(const char *val, size_t len) {
	size_t size_needed = write_pos_ + len;
	Resize(size_needed);

	memcpy(&buffer_[write_pos_], val, len);
	write_pos_ += len;
}

void EQEmu::MemoryBuffer::Read(uchar *buf, size_t len) {
	memcpy(buf, &buffer_[read_pos_], len);
	read_pos_ += len;
}

void EQEmu::MemoryBuffer::Read(char *str) {
	size_t len = strlen((const char*)&buffer_[read_pos_]);
	memcpy(str, &buffer_[read_pos_], len);
	read_pos_ += len;
}
