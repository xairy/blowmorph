#ifndef BLOWMORPH_BASE_INI_FILE_HPP_
#define BLOWMORPH_BASE_INI_FILE_HPP_

#include <string>
#include <map>

namespace bm { namespace ini {

typedef std::map<std::string, std::string> RecordMap;
typedef std::map<std::string, RecordMap> SectionMap;

void Flatten(const SectionMap& in, RecordMap& out);
void UnFlatten(const RecordMap& in, SectionMap& out);

bool LoadINI(const std::string& path, RecordMap& ini);
bool SaveINI(const std::string& path, const RecordMap& ini);

template<class T> T GetValue(const RecordMap& ini, const std::string& key, T defValue = T());
template<> std::string GetValue(const RecordMap& ini, const std::string& key, std::string defValue);
template<> int GetValue(const RecordMap& ini, const std::string& key, int defValue);
template<> bool GetValue(const RecordMap& ini, const std::string& key, bool defValue);

}}; // namespace bm::ini

#endif /*BLOWMORPH_BASE_INI_FILE_HPP_*/
