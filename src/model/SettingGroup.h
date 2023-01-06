#ifndef SETTING_GROUP_H
#define SETTING_GROUP_H

#include "../Any/Any.h"
#include "Setting.h"

struct SettingGroup : public Object {
    String name;
    std::vector<Setting> settings;

    SettingGroup(const bool& isValid = true)
        : m_IsValid(isValid) {}

    SettingGroup(const String& name, const std::vector<Setting>& settings)
        : name(name),
          settings(settings),
          m_IsValid(true) {}

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isString() || !tokens[1].isArray()) {
            m_IsValid = false;
            return;
        }

        name = tokens[0].toString();

        Array array = tokens[1];
        settings.clear();

        for (auto& e : array) {
            if (!e.isObject()) {
                m_IsValid = false;
                return;
            }

            Setting setting = e;
            if (!setting) {
                m_IsValid = false;
                return;
            }

            settings.push_back(setting);
        }
    }

    String toString() const override {
        return stringifyMembers(name, Array::of(settings));
    }

    String serialize() const override {
        return serializeMembers(name, Array::of(settings));
    }

    bool equals(const Object& other) const override {
        const SettingGroup& otherSettingGroup = static_cast<const SettingGroup&>(other);
        return name == otherSettingGroup.name && settings == otherSettingGroup.settings;
    }

    size_t size() const override {
        return 2;
    }

    bool isValid() const override {
        return m_IsValid;
    }

    Object* clone() const override {
        return new SettingGroup(*this);
    }

   private:
    bool m_IsValid;
};

#endif