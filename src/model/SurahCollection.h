#ifndef SURAH_COLLECTION_H
#define SURAH_COLLECTION_H

#include "../vendor/Any/Any.h"

struct SurahCollection : public Object {
    String name;
    uint16_t totalSize;
    uint16_t progress;

    SurahCollection(const bool& isValid = true)
        : m_IsValid(isValid) {}

    SurahCollection(const String& name, const uint16_t& totalSize, const uint16_t& progress)
        : name(name),
          totalSize(totalSize),
          progress(progress),
          m_IsValid(true) {}

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isString() || !tokens[1].isNumber() || !tokens[2].isNumber()) {
            m_IsValid = false;
            return;
        }

        name      = tokens[0].toString();
        totalSize = tokens[1].toInt();
        progress  = tokens[2].toInt();
    }

    String toString() const override {
        return stringifyMembers(name, totalSize, progress);
    }

    String serialize() const override {
        return serializeMembers(name, totalSize, progress);
    }

    bool equals(const Object& other) const override {
        const SurahCollection& otherCollection = static_cast<const SurahCollection&>(other);
        return name == otherCollection.name && totalSize == otherCollection.totalSize && progress == otherCollection.progress;
    }

    size_t size() const override {
        return 3;
    }

    bool isValid() const override {
        return m_IsValid;
    }

    Object* clone() const override {
        return new SurahCollection(*this);
    }

   private:
    bool m_IsValid;
};

#endif