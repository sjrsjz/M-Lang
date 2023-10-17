#pragma once
#include "SectionManager.h"
namespace MLang {
	SectionManager::SectionManager() {}
	SectionManager::~SectionManager() {}
	void SectionManager::Clear() {
		names.clear();
		bins.clear();
	}
	void SectionManager::Ins(const lstring& name, const ByteArray<>& bin, std::optional<bool>different) {
		if (!different.has_value()) different = true;
		if (different) {
			for (int i = 0; i < names.size(); i++) {
				if (names[i] == name) {
					bins[i] = bin;
					return;
				}
			}
		}
		names.push_back(name);
		bins.push_back(bin);
	}
	void SectionManager::Delete(const lstring& name) {
		for (int i = 0; i < names.size(); i++) {
			if (names[i] == name) {
				names.erase(names.begin() + i);
				bins.erase(bins.begin() + i);
				return;
			}
		}
	}
	ByteArray<>& SectionManager::Get(const lstring& name) {
		for (int i = 0; i < names.size(); i++) {
			if (names[i] == name) {
				return bins[i];
			}
		}
		ByteArray<> t = ByteArray(0);
		return t;
	}
	ByteArray<> SectionManager::build() {
		ByteArray tbin;
		ByteArray tbin2;
		size_t p{}, tlength{};
		std::vector<size_t> pos;
		tbin = tbin + names.size();
		for (int i = 0; i < names.size(); i++) {
			tlength += sizeof(size_t) + names[i].size() + sizeof(lchar);
		}
		for (int i = 0; i < names.size(); i++) {
			tbin = tbin + (size_t)(p + tlength + sizeof(size_t) + sizeof(lchar));
			tbin = tbin + names[i];
			tbin = tbin + (lchar)0;
			p += sizeof(size_t) + bins[i].size;
			tbin2 = tbin2 + bins[i].size;
			tbin2 = tbin2 + bins[i];

		}
		return tbin + tbin2;
	}
	void SectionManager::translate( ByteArray<> bin) {
		size_t i = bin.Get<size_t>(0);
		size_t pos = sizeof(size_t);
		names.clear();
		bins.clear();
		for (size_t j = 0; j < i; j++) {
			size_t tpos = bin.Get<size_t>(pos) - sizeof(lchar);
			pos += sizeof(size_t);
			lstring t1;
			size_t k = 0;
			for (;pos + k < bin.size; k++) {
				if (bin.Get<lchar>(pos + k) == 0) break;
			}
			t1.resize(k, 0);
			memcpy((void*)t1.c_str(),&(bin.Get<char>(pos)),k);
			pos += sizeof(lchar) + k;
			names.push_back(t1);
			bins.push_back(bin.SubByteArray(tpos+sizeof(size_t),bin.Get<size_t>(tpos)));
		}
	}
}