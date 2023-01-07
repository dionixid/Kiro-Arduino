#include "TinyDB.h"

TinyDB::TinyDB() : fs(NULL) {}

TinyDB::~TinyDB() {}

/**
 * @brief Initialize the file system.
 * This method must be called before using the TinyDB instance.
 * 
 * @param fs is the file system to be used by this instance.
 */
void TinyDB::begin(FS &fs) {
    this->fs = &fs;
}

/**
 * @brief Check if a file exists.
 * 
 * @param key is the file name.
 * @return true if the file exists. false otherwise.
 */
bool TinyDB::contains(const String &key) {
    if(!fs) return false;
    File file = fs->open(validate(key).c_str(), FILE_READ);
    if (file && !file.isDirectory()) {
        file.close();
        return true;
    }
    return false;
}

/**
 * @brief Remove a file from the file system.
 * 
 * @param key is the file name.
 * @return true if the file was removed. false otherwise.
 */
bool TinyDB::remove(const String &key) {
    if(!fs) return false;
    return fs->remove(validate(key).c_str());
}

/**
 * @brief List all files in the file system.
 * 
 * @return An array of file names.
 */
Array TinyDB::listFiles() {
    Array v;
    if(!fs) {
        return v;
    }
    
    File root = fs->open("/", FILE_READ);
    root.rewindDirectory();
    File entry = root.openNextFile();

    while (entry) {
        String str(entry.name());
        str.toLowerCase();
        v.push(str);
        entry.close();
        entry = root.openNextFile();
    }

    root.close();
    return v;
}

/**
 * @brief Validate the file name.
 * Add a leading slash if it is missing.
 * 
 * @param key is the file name.
 * @return A valid file name.
 */
String TinyDB::validate(const String &key) {
    if (!key.startsWith("/")) {
        return "/" + key;
    }
    return key;
}

/**
 * @brief Write a string to the file system.
 * 
 * @param key is the file name.
 * @param value is the string to be written.
 * @return true if the file was written. false otherwise.
 */
bool TinyDB::write(const String &key, String value) {
    if(!fs) return false;
    File file = fs->open(validate(key).c_str(), FILE_WRITE);
    if (!file || file.isDirectory()) {
        return false;
    }
    int res = file.print(value);
    file.close();
    return res;
}

/**
 * @brief Append a string to the file system.
 * 
 * @param key is the file name.
 * @param value is the string to be appended.
 * @return true if the file was appended. false otherwise.
 */
bool TinyDB::append(const String &key, String value) {
    if(!fs) return false;
    File file = fs->open(validate(key).c_str(), FILE_APPEND);
    if (!file || file.isDirectory()) {
        return false;
    }
    int res = file.print(value);
    file.close();
    return res;
}

/**
 * @brief Read a string from the file system.
 * 
 * @param key is the file name.
 * @return The string read from the file.
 */
String TinyDB::read(const String &key) {
    if(!fs) return "";
    File file = fs->open(validate(key).c_str(), FILE_READ);
    if (!file || file.isDirectory()) {
        return "";
    }
    String res = file.readString();
    file.close();
    return res;
}

/**
 * @brief Check if a file is writable.
 * e.g. if it is not a directory.
 * If the file exists, it will be deleted.
 * 
 * @param key is the file name.
 * @return true if the file is writable. false otherwise.
 */
bool TinyDB::isWritable(const String &key) {
    if(!fs) return false;
    File file = fs->open(validate(key).c_str(), FILE_READ);
    if (file && !file.isDirectory()) {
        file.close();
        return fs->remove(validate(key).c_str());
    }
    return true;
}
