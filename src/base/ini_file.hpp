#ifndef BLOWMORPH_BASE_INI_FILE_HPP_
#define BLOWMORPH_BASE_INI_FILE_HPP_

#include <string>
#include <map>

#include <base/base.hpp>

namespace bm { namespace ini {

typedef std::map<std::string, std::string> RecordMap;
typedef std::map<std::string, RecordMap> SectionMap;

BM_BASE_DECL void Flatten(const SectionMap& in, RecordMap& out);
BM_BASE_DECL void UnFlatten(const RecordMap& in, SectionMap& out);

BM_BASE_DECL bool LoadINI(const std::string& path, RecordMap& ini);
BM_BASE_DECL bool SaveINI(const std::string& path, const RecordMap& ini);

template<class T> T GetValue(const RecordMap& ini, const std::string& key, T defValue = T());
template<> BM_BASE_DECL std::string GetValue(const RecordMap& ini, const std::string& key, std::string defValue);
template<> BM_BASE_DECL int GetValue(const RecordMap& ini, const std::string& key, int defValue);
template<> BM_BASE_DECL bool GetValue(const RecordMap& ini, const std::string& key, bool defValue);
template<> BM_BASE_DECL float GetValue(const RecordMap& ini, const std::string& key, float defValue);
template<> BM_BASE_DECL double GetValue(const RecordMap& ini, const std::string& key, double defValue);

}}; // namespace bm::ini

#endif /*BLOWMORPH_BASE_INI_FILE_HPP_*/
