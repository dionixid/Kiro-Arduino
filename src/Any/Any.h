#ifndef ANY_H
#define ANY_H

#include <Arduino.h>
#include <Print.h>
#include <Printable.h>

#include <iterator>
#include <type_traits>
#include <vector>

class Any;

namespace AnyParser {
const char OBJECT_OPEN_BRACKET  = '{';
const char OBJECT_CLOSE_BRACKET = '}';
const char ARRAY_OPEN_BRACKET   = '[';
const char ARRAY_CLOSE_BRACKET  = ']';
const char STRING_BRACKET       = '"';
const char SEPARATOR            = ',';

const String TRUE                  = "true";
const String FALSE                 = "false";
const String NULL_                 = "null";
const String ESCAPE_STRING_BRACKET = "\\\"";

template <typename T, template <typename...> class R>
struct is_type_of : std::false_type {};

template <template <typename...> class R, typename... T>
struct is_type_of<R<T...>, R> : std::true_type {};

std::vector<Any> parse(const String &str);
}  // namespace AnyParser

class Object : public Printable {
   public:
    /**
     * @brief Get the String representation of this Object.
     * This method is different from the serialize() method, and cannot be used interchangeably.
     * An inheriting class MUST override this method to provide a correct String representation.
     *
     * @return A String representation of this Object.
     */
    virtual String toString() const = 0;

    /**
     * @brief Serialize this Object into a String.
     * This method is different from the toString() method, and cannot be used interchangeably.
     * An inheriting class MUST override this method to provide a correct serialization.
     *
     * @return A Serialized String representation of this Object.
     */
    virtual String serialize() const = 0;

    /**
     * @brief Check the equality of this Object with another Object.
     * An inheriting class MUST override this method to provide a correct equality check.
     *
     * @param other another Object.
     * @return true if the two objects are equal. false otherwise.
     */
    virtual bool equals(const Object &other) const = 0;

    /**
     * @brief Check the truthiness of this Object.
     * An inheriting class MUST override this method to provide a correct truthiness check.
     *
     * @return true if this Object is truthy. false otherwise.
     */
    virtual bool isValid() const = 0;

    /**
     * @brief Get the size of this Object.
     * An inheriting class MUST override this method to provide a correct size.
     *
     * @return The size of this Object.
     */
    virtual size_t size() const = 0;

   protected:
    Object();
    virtual ~Object();

    /**
     * @brief Construct a new Object from a serialized Object.
     * An inheriting class MUST override this method to provide a correct deserialization.
     * This method is protected, and can only be called by an inheriting class.
     *
     * @param tokens the deserialized members of this Object.
     */
    virtual void constructor(const std::vector<Any> &tokens) = 0;

    /**
     * @brief Create a copy of this Object.
     * The caller is responsible for freeing the memory allocated by this method.
     * An inheriting class MUST override this method to provide a correct copy.
     *
     * @return A pointer to the newly created object.
     */
    virtual Object *clone() const = 0;

    /**
     * @brief Get the String representation of this Object.
     * An inheriting class should use this method to stringify its members.
     * This method is usually called inside overridden toString() method.
     * This method is protected, and can only be called by an inheriting class.
     *
     * @tparam T
     * @param args
     * @return String
     */
    template <typename... T>
    String stringifyMembers(T... args) const {
        String result = String(AnyParser::OBJECT_OPEN_BRACKET);
        size_t size   = sizeof...(args);
        Any members[] = {args...};

        if (size > 0) {
            result += members[0].toString();
        }

        for (size_t i = 1; i < size; i++) {
            result += AnyParser::SEPARATOR;
            result += members[i].toString();
        }

        result += String(AnyParser::OBJECT_CLOSE_BRACKET);
        return result;
    }

    /**
     * @brief Serialize members into a String.
     * An inheriting class should use this method to serialize its members.
     * This method is usually called inside overridden serialize() method.
     * This method is protected, and can only be called by an inheriting class.
     *
     * @tparam T
     * @param args
     * @return String
     */
    template <typename... T>
    String serializeMembers(T... args) const {
        String result = String(AnyParser::OBJECT_OPEN_BRACKET);
        size_t size   = sizeof...(args);
        Any members[] = {args...};

        if (size > 0) {
            result += members[0].serialize();
        }

        for (size_t i = 1; i < size; i++) {
            result += AnyParser::SEPARATOR;
            result += members[i].serialize();
        }

        result += String(AnyParser::OBJECT_CLOSE_BRACKET);
        return result;
    }

   public:
    virtual size_t printTo(Print &p) const;

    bool operator==(const Object &other) const;
    bool operator!=(const Object &other) const;
    operator bool() const;

    friend class Any;
};

class Array : public Printable {
   public:
    Array();
    Array(const Array &other);
    Array(const Any &other);

    template <typename... T>
    Array(T... args) {
        push(args...);
    }

    template <typename... T>
    Array &push(T... args) {
        int dummy[] = {(m_Data.push_back(args), 0)...};
        (void)dummy;
        return *this;
    }

    Any &operator[](const uint16_t &index);
    const Any &operator[](const uint16_t &index) const;

    Any &get(const uint16_t &index);
    const Any &get(const uint16_t &index) const;

    bool equals(const Array &other) const;
    bool operator==(const Array &other) const;
    bool operator!=(const Array &other) const;

    String toString() const;
    String serialize() const;
    size_t size() const;
    size_t lastIndex() const;

    bool contains(const Any &e) const;
    bool isEmpty() const;
    bool isNotEmpty() const;

    void remove(const size_t &index, const size_t &count = 1);
    void clear();

    std::vector<Any>::iterator begin();
    std::vector<Any>::iterator end();

    operator bool() const;

    size_t printTo(Print &p) const;

    template <typename T>
    static Array of(const std::vector<T> &v) {
        Array result;
        result.m_Data.reserve(v.size());
        for (const T &e : v) {
            result.m_Data.push_back(e);
        }
        return result;
    }

    friend class Any;

   private:
    std::vector<Any> m_Data;
    Array *_clone() const;
};

class Any : public Printable {
   public:
    enum class Type : uint8_t {
        Object,
        Array,
        String,
        Float,
        Integer,
        Boolean,
        Null
    };

    Any();
    ~Any();

    Any(const Any &other);
    Any(Any &&other);
    Any(const char &);
    Any(const signed char &);
    Any(const short &);
    Any(const int &);
    Any(const long &);
    Any(const long long &);
    Any(const unsigned char &);
    Any(const unsigned short &);
    Any(const unsigned int &);
    Any(const unsigned long &);
    Any(const unsigned long long &);
    Any(const float &);
    Any(const double &);
    Any(const String &);
    Any(const char *);
    Any(const bool &);
    Any(const Array &);

    template <typename T, typename = typename std::enable_if<std::is_base_of<Object, T>::value>::type>
    Any(const T &e) {
        m_Type        = Type::Object;
        m_Data.object = new T(e);
        _validate();
    }

    operator char() const;
    operator signed char() const;
    operator short() const;
    operator int() const;
    operator long() const;
    operator long long() const;
    operator unsigned char() const;
    operator unsigned short() const;
    operator unsigned int() const;
    operator unsigned long() const;
    operator unsigned long long() const;
    operator float() const;
    operator double() const;
    operator bool() const;
    operator String() const;
    operator Array &();

    template <typename T, typename = typename std::enable_if<std::is_base_of<Object, T>::value>::type>
    operator T &() const {
        if (m_Type == Type::String && m_IsUnsetObject) {
            m_IsUnsetObject = false;
            String str      = *m_Data.string;
            _release();
            m_Data = new T();
            if (m_Data.object) {
                m_Data.object->constructor(AnyParser::parse(str));
                m_Type = Type::Object;
            }
            _validate();
        }

        if (m_Type != Type::Object) {
            _release();
            m_Data = new T();
            if (m_Data.object) {
                m_Data.object->constructor(std::vector<Any>());
                m_Type = Type::Object;
            }
            _validate();
        }

        return *static_cast<T *>(m_Data.object);
    }

    Any &operator=(const Any &e);
    Any &operator=(Any &&e);

    Any &operator[](const Any &index);
    Any &operator[](const char &index);
    Any &operator[](const signed char &index);
    Any &operator[](const short &index);
    Any &operator[](const int &index);
    Any &operator[](const long &index);
    Any &operator[](const unsigned char &index);
    Any &operator[](const unsigned short &index);
    Any &operator[](const unsigned int &index);
    Any &operator[](const unsigned long &index);

    const Any &operator[](const Any &index) const;
    const Any &operator[](const char &index) const;
    const Any &operator[](const signed char &index) const;
    const Any &operator[](const short &index) const;
    const Any &operator[](const int &index) const;
    const Any &operator[](const long &index) const;
    const Any &operator[](const unsigned char &index) const;
    const Any &operator[](const unsigned short &index) const;
    const Any &operator[](const unsigned int &index) const;
    const Any &operator[](const unsigned long &index) const;

    bool operator==(const Any &e) const;
    bool operator==(const char &e) const;
    bool operator==(const signed char &e) const;
    bool operator==(const short &e) const;
    bool operator==(const int &e) const;
    bool operator==(const long &e) const;
    bool operator==(const long long &e) const;
    bool operator==(const unsigned char &e) const;
    bool operator==(const unsigned short &e) const;
    bool operator==(const unsigned int &e) const;
    bool operator==(const unsigned long &e) const;
    bool operator==(const unsigned long long &e) const;
    bool operator==(const float &e) const;
    bool operator==(const double &e) const;
    bool operator==(const bool &e) const;
    bool operator==(const char *e) const;
    bool operator==(const String &e) const;

    bool operator!=(const Any &e) const;
    bool operator!=(const char &e) const;
    bool operator!=(const signed char &e) const;
    bool operator!=(const short &e) const;
    bool operator!=(const int &e) const;
    bool operator!=(const long &e) const;
    bool operator!=(const long long &e) const;
    bool operator!=(const unsigned char &e) const;
    bool operator!=(const unsigned short &e) const;
    bool operator!=(const unsigned int &e) const;
    bool operator!=(const unsigned long &e) const;
    bool operator!=(const unsigned long long &e) const;
    bool operator!=(const float &e) const;
    bool operator!=(const double &e) const;
    bool operator!=(const bool &e) const;
    bool operator!=(const char *e) const;
    bool operator!=(const String &e) const;

    bool operator<(const Any &e) const;
    bool operator<(const char &e) const;
    bool operator<(const signed char &e) const;
    bool operator<(const short &e) const;
    bool operator<(const int &e) const;
    bool operator<(const long &e) const;
    bool operator<(const long long &e) const;
    bool operator<(const unsigned char &e) const;
    bool operator<(const unsigned short &e) const;
    bool operator<(const unsigned int &e) const;
    bool operator<(const unsigned long &e) const;
    bool operator<(const unsigned long long &e) const;
    bool operator<(const float &e) const;
    bool operator<(const double &e) const;
    bool operator<(const bool &e) const;
    bool operator<(const char *e) const;
    bool operator<(const String &e) const;

    bool operator>(const Any &e) const;
    bool operator>(const char &e) const;
    bool operator>(const signed char &e) const;
    bool operator>(const short &e) const;
    bool operator>(const int &e) const;
    bool operator>(const long &e) const;
    bool operator>(const long long &e) const;
    bool operator>(const unsigned char &e) const;
    bool operator>(const unsigned short &e) const;
    bool operator>(const unsigned int &e) const;
    bool operator>(const unsigned long &e) const;
    bool operator>(const unsigned long long &e) const;
    bool operator>(const float &e) const;
    bool operator>(const double &e) const;
    bool operator>(const bool &e) const;
    bool operator>(const char *e) const;
    bool operator>(const String &e) const;

    bool operator<=(const Any &e) const;
    bool operator<=(const char &e) const;
    bool operator<=(const signed char &e) const;
    bool operator<=(const short &e) const;
    bool operator<=(const int &e) const;
    bool operator<=(const long &e) const;
    bool operator<=(const long long &e) const;
    bool operator<=(const unsigned char &e) const;
    bool operator<=(const unsigned short &e) const;
    bool operator<=(const unsigned int &e) const;
    bool operator<=(const unsigned long &e) const;
    bool operator<=(const unsigned long long &e) const;
    bool operator<=(const float &e) const;
    bool operator<=(const double &e) const;
    bool operator<=(const bool &e) const;
    bool operator<=(const char *e) const;
    bool operator<=(const String &e) const;

    bool operator>=(const Any &e) const;
    bool operator>=(const char &e) const;
    bool operator>=(const signed char &e) const;
    bool operator>=(const short &e) const;
    bool operator>=(const int &e) const;
    bool operator>=(const long &e) const;
    bool operator>=(const long long &e) const;
    bool operator>=(const unsigned char &e) const;
    bool operator>=(const unsigned short &e) const;
    bool operator>=(const unsigned int &e) const;
    bool operator>=(const unsigned long &e) const;
    bool operator>=(const unsigned long long &e) const;
    bool operator>=(const float &e) const;
    bool operator>=(const double &e) const;
    bool operator>=(const bool &e) const;
    bool operator>=(const char *e) const;
    bool operator>=(const String &e) const;

    Any operator+(const Any &e) const;
    Any operator+(const char &e) const;
    Any operator+(const signed char &e) const;
    Any operator+(const short &e) const;
    Any operator+(const int &e) const;
    Any operator+(const long &e) const;
    Any operator+(const long long &e) const;
    Any operator+(const unsigned char &e) const;
    Any operator+(const unsigned short &e) const;
    Any operator+(const unsigned int &e) const;
    Any operator+(const unsigned long &e) const;
    Any operator+(const unsigned long long &e) const;
    Any operator+(const float &e) const;
    Any operator+(const double &e) const;
    Any operator+(const bool &e) const;
    Any operator+(const char *e) const;
    Any operator+(const String &e) const;

    Any operator-(const Any &e) const;
    Any operator-(const char &e) const;
    Any operator-(const signed char &e) const;
    Any operator-(const short &e) const;
    Any operator-(const int &e) const;
    Any operator-(const long &e) const;
    Any operator-(const long long &e) const;
    Any operator-(const unsigned char &e) const;
    Any operator-(const unsigned short &e) const;
    Any operator-(const unsigned int &e) const;
    Any operator-(const unsigned long &e) const;
    Any operator-(const unsigned long long &e) const;
    Any operator-(const float &e) const;
    Any operator-(const double &e) const;

    Any operator*(const Any &e) const;
    Any operator*(const char &e) const;
    Any operator*(const signed char &e) const;
    Any operator*(const short &e) const;
    Any operator*(const int &e) const;
    Any operator*(const long &e) const;
    Any operator*(const long long &e) const;
    Any operator*(const unsigned char &e) const;
    Any operator*(const unsigned short &e) const;
    Any operator*(const unsigned int &e) const;
    Any operator*(const unsigned long &e) const;
    Any operator*(const unsigned long long &e) const;
    Any operator*(const float &e) const;
    Any operator*(const double &e) const;

    Any operator/(const Any &e) const;
    Any operator/(const char &e) const;
    Any operator/(const signed char &e) const;
    Any operator/(const short &e) const;
    Any operator/(const int &e) const;
    Any operator/(const long &e) const;
    Any operator/(const long long &e) const;
    Any operator/(const unsigned char &e) const;
    Any operator/(const unsigned short &e) const;
    Any operator/(const unsigned int &e) const;
    Any operator/(const unsigned long &e) const;
    Any operator/(const unsigned long long &e) const;
    Any operator/(const float &e) const;
    Any operator/(const double &e) const;

    Any operator%(const Any &e) const;
    Any operator%(const char &e) const;
    Any operator%(const signed char &e) const;
    Any operator%(const short &e) const;
    Any operator%(const int &e) const;
    Any operator%(const long &e) const;
    Any operator%(const long long &e) const;
    Any operator%(const unsigned char &e) const;
    Any operator%(const unsigned short &e) const;
    Any operator%(const unsigned int &e) const;
    Any operator%(const unsigned long &e) const;
    Any operator%(const unsigned long long &e) const;
    Any operator%(const float &e) const;
    Any operator%(const double &e) const;

    Any &operator+=(const Any &e);
    Any &operator+=(const char &e);
    Any &operator+=(const signed char &e);
    Any &operator+=(const short &e);
    Any &operator+=(const int &e);
    Any &operator+=(const long &e);
    Any &operator+=(const long long &e);
    Any &operator+=(const unsigned char &e);
    Any &operator+=(const unsigned short &e);
    Any &operator+=(const unsigned int &e);
    Any &operator+=(const unsigned long &e);
    Any &operator+=(const unsigned long long &e);
    Any &operator+=(const float &e);
    Any &operator+=(const double &e);
    Any &operator+=(const bool &e);
    Any &operator+=(const char *e);
    Any &operator+=(const String &e);

    Any &operator-=(const Any &e);
    Any &operator-=(const char &e);
    Any &operator-=(const signed char &e);
    Any &operator-=(const short &e);
    Any &operator-=(const int &e);
    Any &operator-=(const long &e);
    Any &operator-=(const long long &e);
    Any &operator-=(const unsigned char &e);
    Any &operator-=(const unsigned short &e);
    Any &operator-=(const unsigned int &e);
    Any &operator-=(const unsigned long &e);
    Any &operator-=(const unsigned long long &e);
    Any &operator-=(const float &e);
    Any &operator-=(const double &e);

    Any &operator*=(const Any &e);
    Any &operator*=(const char &e);
    Any &operator*=(const signed char &e);
    Any &operator*=(const short &e);
    Any &operator*=(const int &e);
    Any &operator*=(const long &e);
    Any &operator*=(const long long &e);
    Any &operator*=(const unsigned char &e);
    Any &operator*=(const unsigned short &e);
    Any &operator*=(const unsigned int &e);
    Any &operator*=(const unsigned long &e);
    Any &operator*=(const unsigned long long &e);
    Any &operator*=(const float &e);
    Any &operator*=(const double &e);

    Any &operator/=(const Any &e);
    Any &operator/=(const char &e);
    Any &operator/=(const signed char &e);
    Any &operator/=(const short &e);
    Any &operator/=(const int &e);
    Any &operator/=(const long &e);
    Any &operator/=(const long long &e);
    Any &operator/=(const unsigned char &e);
    Any &operator/=(const unsigned short &e);
    Any &operator/=(const unsigned int &e);
    Any &operator/=(const unsigned long &e);
    Any &operator/=(const unsigned long long &e);
    Any &operator/=(const float &e);
    Any &operator/=(const double &e);

    Any &operator%=(const Any &e);
    Any &operator%=(const char &e);
    Any &operator%=(const signed char &e);
    Any &operator%=(const short &e);
    Any &operator%=(const int &e);
    Any &operator%=(const long &e);
    Any &operator%=(const long long &e);
    Any &operator%=(const unsigned char &e);
    Any &operator%=(const unsigned short &e);
    Any &operator%=(const unsigned int &e);
    Any &operator%=(const unsigned long &e);
    Any &operator%=(const unsigned long long &e);
    Any &operator%=(const float &e);
    Any &operator%=(const double &e);

    Any &operator++();
    Any operator++(int);
    Any &operator--();
    Any operator--(int);

    int64_t toInt() const;
    float toFloat() const;
    double toDouble() const;
    bool toBool() const;
    String toString() const;
    const char *c_str() const;

    String serialize() const;

    Type getType() const;
    String getTypeName() const;
    size_t size() const;
    size_t lastIndex() const;

    void remove(const size_t &index, const size_t &count = 1);
    void clear();

    bool contains(const Any &e) const;
    bool isNull() const;
    bool isNotNull() const;
    bool isEmpty() const;
    bool isNotEmpty() const;
    bool isNegative() const;

    bool isObject() const;
    bool isArray() const;
    bool isString() const;
    bool isNumber() const;
    bool isBigNumber() const;
    bool isBool() const;

    size_t printTo(Print &p) const;

    std::vector<Any>::iterator begin();
    std::vector<Any>::iterator end();

    static Any parse(const String &str);

    template <typename T>
    T as() {
        static_assert(
            std::is_base_of<Object, T>::value || std::is_same<Array, T>::value || std::is_same<String, T>::value
                || std::is_integral<T>::value || std::is_floating_point<T>::value || std::is_same<bool, T>::value,
            "T must be an Array, String, Integer, Floating, Boolean or a class derived from Object."
        );
        return *this;
    }

    template <typename T>
    const T as() const {
        static_assert(
            std::is_base_of<Object, T>::value || std::is_same<Array, T>::value || std::is_same<String, T>::value
                || std::is_integral<T>::value || std::is_floating_point<T>::value || std::is_same<bool, T>::value,
            "T must be an Array, String, Integer, Floating, Boolean or a class derived from Object."
        );
        return *this;
    }

   private:
    union data_t {
        mutable String *string = NULL;
        mutable Object *object;
        mutable Array *array;
        mutable double floating;
        mutable int64_t integer;
        mutable bool boolean;

        data_t() {}
        ~data_t() {}

        data_t(String *value)
            : string(value) {}

        data_t(Object *object)
            : object(object) {}

        data_t(Array *array)
            : array(array) {}

        data_t(double floating)
            : floating(floating) {}

        data_t(int64_t integer)
            : integer(integer) {}

        data_t(bool boolean)
            : boolean(boolean) {}

        data_t &operator=(String *value) {
            string = value;
            return *this;
        }

        data_t &operator=(Object *object) {
            this->object = object;
            return *this;
        }

        data_t &operator=(Array *array) {
            this->array = array;
            return *this;
        }

        data_t &operator=(const double &floating) {
            this->floating = floating;
            return *this;
        }

        data_t &operator=(const int64_t &integer) {
            this->integer = integer;
            return *this;
        }

        data_t &operator=(const bool &boolean) {
            this->boolean = boolean;
            return *this;
        }
    };

    mutable Type m_Type;
    mutable data_t m_Data;
    mutable bool m_IsUnsetObject = false;

    void _release() const;
    void _validate() const;
    int _compareTo(const Any &other) const;
};

namespace AnyParser {
int16_t findClosingObjectBracket(const String &src, const int16_t &start);
int16_t findClosingArrayBracket(const String &src, const int16_t &start);
int16_t findClosingQuote(const String &src, const int16_t &start);

String toString(const float &value);
String toString(const double &value);
String toString(const int64_t &value);
int64_t parseInt(const String &str);

bool isArray(const String &str);
bool isObject(const String &str);
bool isString(const String &str);
bool isFloat(const String &str);
bool isNumber(const String &str);
bool isLiteral(const String &str);
String removeInsignificantZeros(const String &str);

Any parseLiteral(const String &str);
String serialize(const String &str);
};  // namespace AnyParser

#endif