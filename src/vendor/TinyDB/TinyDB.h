#ifndef TINYDB_H
#define TINYDB_H

#include <Arduino.h>
#include <FS.h>

#include "../Any/Any.h"

#ifndef FILE_WRITE
#define FILE_WRITE "w"
#endif

#ifndef FILE_READ
#define FILE_READ "r"
#endif

#ifndef FILE_APPEND
#define FILE_APPEND "a"
#endif

class TinyDB {
   public:
    TinyDB();
    ~TinyDB();

    void begin(FS &fs);
    bool contains(const String &key);
    bool remove(const String &key);
    Array listFiles();

    /**
     * @brief Write an Any object to the file system.
     *
     * @param key is the file name.
     * @param value is the Any object.
     * @return true if the file was written. false otherwise.
     */
    bool put(const String &key, const Any &value) {
        return write(key, value.serialize());
    }

    /**
     * @brief Read an Any object from the file system.
     * If the file does not exist, a Null is returned.
     * The caller should check the type before using the value.
     *
     * @param key is the file name.
     * @return an Any object.
     */
    Any get(const String &key) {
        return Any::parse(read(key));
    }

   private:
    fs::FS *fs;
    String validate(const String &key);
    bool write(const String &key, String content);
    bool append(const String &key, String content);
    String read(const String &key);
    bool isWritable(const String &key);
};

#endif
