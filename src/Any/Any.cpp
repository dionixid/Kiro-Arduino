#include "Any.h"

/*-----------------------------------------------------------
 * OBJECT CLASS IMPLEMENTATION
 *----------------------------------------------------------*/

/**
 * @brief Construct a new Object.
 * This constructor is used to create an empty object.
 * This constructor is protected, and can only be called by an inheriting class.
 */
Object::Object() {}

/**
 * @brief Destroy the Object.
 * This destructor is protected, and can only be called by an inheriting class.
 *
 */
Object::~Object() {}

/**
 * @brief Print this Object to a Print object.
 * This method is used by the Arduino Print class.
 * An inheriting class could override this method to provide a custom print method.
 * By default, the output of this method is the same as the output of the toString() method.
 *
 * @param p an arduino Print object.
 * @return The number of bytes written.
 */
size_t Object::printTo(Print &p) const {
    return p.print(toString());
}

/**
 * @brief Check the equality of this Object with another Object.
 *
 * @param other another Object.
 * @return true if the two objects are equal. false otherwise.
 */
bool Object::operator==(const Object &other) const {
    return equals(other);
}

/**
 * @brief Check the inequality of this Object with another Object.
 *
 * @param other another Object.
 * @return true if the two objects are not equal. false otherwise.
 */
bool Object::operator!=(const Object &other) const {
    return !equals(other);
}

/**
 * @brief Check the truthiness of this Object.
 *
 * @return true if this Object is truthy. false otherwise.
 */
Object::operator bool() const {
    return isValid();
}

/*-----------------------------------------------------------
 * ARRAY CLASS IMPLEMENTATION
 *----------------------------------------------------------*/

/**
 * @brief Construct a new Array.
 * This constructor is used to create an empty array.
 *
 */
Array::Array()
    : m_Data(std::vector<Any>()) {}

/**
 * @brief Construct a new Array from another Array.
 *
 * @param other
 */
Array::Array(const Array &other)
    : m_Data(other.m_Data) {}

Any &Array::operator[](const uint16_t &index) {
    return m_Data.at(index);
}

const Any &Array::operator[](const uint16_t &index) const {
    return m_Data.at(index);
}

Any &Array::get(const uint16_t &index) {
    return operator[](index);
}

const Any &Array::get(const uint16_t &index) const {
    return operator[](index);
}

/**
 * @brief Check if this array is equal to another array.
 *
 * @param other is the other array to compare to.
 * @return true if the two arrays are equal. false otherwise.
 */
bool Array::equals(const Array &other) const {
    return m_Data == other.m_Data;
}

bool Array::operator==(const Array &other) const {
    return equals(other);
}

bool Array::operator!=(const Array &other) const {
    return !equals(other);
}

/**
 * @brief Get the String representation of this Array.
 * This method is different from the serialize() method, and cannot be used interchangeably.
 *
 * @return String
 */
String Array::toString() const {
    String result = String(AnyParser::ARRAY_OPEN_BRACKET);

    if (m_Data.size() > 0) {
        result += m_Data[0].toString();
    }

    for (int i = 1; i < m_Data.size(); i++) {
        result += AnyParser::SEPARATOR;
        result += m_Data[i].toString();
    }

    result += String(AnyParser::ARRAY_CLOSE_BRACKET);
    return result;
}

/**
 * @brief Serialize this Array into a String.
 * This method is different from the toString() method, and cannot be used interchangeably.
 *
 * @return String
 */
String Array::serialize() const {
    String result = String(AnyParser::ARRAY_OPEN_BRACKET);

    if (m_Data.size() > 0) {
        result += m_Data[0].serialize();
    }

    for (int i = 1; i < m_Data.size(); i++) {
        result += AnyParser::SEPARATOR;
        result += m_Data[i].serialize();
    }

    result += String(AnyParser::ARRAY_CLOSE_BRACKET);
    return result;
}

/**
 * @brief Get the size of this Array.
 *
 * @return The size of this Array.
 */
size_t Array::size() const {
    return m_Data.size();
}

/**
 * @brief Get the last index of this Array.
 * The last index of an array is the size of the array minus 1.
 * If the array is empty, the last index is -1.
 *
 * @return The last index of this Array.
 */
size_t Array::lastIndex() const {
    return m_Data.size() - 1;
}

/**
 * @brief Remove elements from this Array.
 *
 * @param index the index of the first element to remove.
 * @param count the number of elements to be removed.
 */
void Array::remove(const size_t &index, const size_t &count) {
    if (index < m_Data.size() && count > 0 && index + count <= m_Data.size()) {
        m_Data.erase(m_Data.begin() + index, m_Data.begin() + index + count);
    }
}

/**
 * @brief Remove all elements of this Array.
 *
 */
void Array::clear() {
    m_Data.clear();
}

/**
 * @brief Check if this Array contains a given element.
 *
 * @param e the element to be checked.
 * @return true if this Array contains the given element. false otherwise.
 */
bool Array::contains(const Any &e) const {
    for (int i = 0; i < m_Data.size(); i++) {
        if (m_Data[i] == e) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Check if this Array is empty.
 *
 * @return true if this Array is empty. false otherwise.
 */
bool Array::isEmpty() const {
    return m_Data.empty();
}

/**
 * @brief Check if this Array is not empty.
 *
 * @return true if this Array is not empty. false otherwise.
 */
bool Array::isNotEmpty() const {
    return !m_Data.empty();
}

/**
 * @brief Get the iterator to the beginning of this Array.
 *
 * @return The iterator to the beginning of this Array.
 */
std::vector<Any>::iterator Array::begin() {
    return m_Data.begin();
}

/**
 * @brief Get the iterator to the end of this Array.
 *
 * @return The iterator to the end of this Array.
 */
std::vector<Any>::iterator Array::end() {
    return m_Data.end();
}

/**
 * @brief Get the boolean representation of this array.
 *
 * @return true if this array is not empty. false otherwise.
 */
Array::operator bool() const {
    return m_Data.size() > 0;
}

/**
 * @brief Print this Array to a Print object.
 * This method is used by the Arduino Print class.
 * The output of this method is the same as the output of the toString() method.
 *
 * @param p an Arduino Print object.
 * @return size_t
 */
size_t Array::printTo(Print &p) const {
    return p.print(toString());
}

/**
 * @brief Create a copy of this Array.
 * The caller is responsible for freeing the memory allocated by this method.
 *
 * @return Array*
 */
Array *Array::_clone() const {
    return new Array(*this);
}

/*-----------------------------------------------------------
 * ANY CLASS IMPLEMENTATION
 *----------------------------------------------------------*/

Any::Any()
    : m_Type(Type::Null),
      m_IsUnsetObject(false) {}

Any::~Any() {
    _release();
}

Any::Any(const Any &other) {
    m_Type          = other.m_Type;
    m_IsUnsetObject = other.m_IsUnsetObject;

    switch (m_Type) {
        case Type::Object: {
            m_Data.object = other.m_Data.object->clone();
            break;
        }
        case Type::Array: {
            m_Data.array = other.m_Data.array->_clone();
            break;
        }
        case Type::String: {
            m_Data.string = new String(*other.m_Data.string);
            break;
        }
        case Type::Integer: {
            m_Data.integer = other.m_Data.integer;
            break;
        }
        case Type::Float: {
            m_Data.floating = other.m_Data.floating;
            break;
        }
        case Type::Boolean: {
            m_Data.boolean = other.m_Data.boolean;
            break;
        }
    }

    _validate();
}

Any::Any(Any &&other)
    : m_Type(other.m_Type),
      m_IsUnsetObject(other.m_IsUnsetObject),
      m_Data(other.m_Data) {
    other.m_Type          = Type::Null;
    other.m_IsUnsetObject = false;
    other.m_Data.string   = NULL;
    _validate();
}

Any::Any(const char &value)
    : m_Type(Type::Integer),
      m_IsUnsetObject(false),
      m_Data(static_cast<int64_t>(value)) {}

Any::Any(const signed char &value)
    : m_Type(Type::Integer),
      m_IsUnsetObject(false),
      m_Data(static_cast<int64_t>(value)) {}

Any::Any(const short &value)
    : m_Type(Type::Integer),
      m_IsUnsetObject(false),
      m_Data(static_cast<int64_t>(value)) {}

Any::Any(const int &value)
    : m_Type(Type::Integer),
      m_IsUnsetObject(false),
      m_Data(static_cast<int64_t>(value)) {}

Any::Any(const long &value)
    : m_Type(Type::Integer),
      m_IsUnsetObject(false),
      m_Data(static_cast<int64_t>(value)) {}

Any::Any(const long long &value)
    : m_Type(Type::Integer),
      m_IsUnsetObject(false),
      m_Data(static_cast<int64_t>(value)) {}

Any::Any(const unsigned char &value)
    : m_Type(Type::Integer),
      m_IsUnsetObject(false),
      m_Data(static_cast<int64_t>(value)) {}

Any::Any(const unsigned short &value)
    : m_Type(Type::Integer),
      m_IsUnsetObject(false),
      m_Data(static_cast<int64_t>(value)) {}

Any::Any(const unsigned int &value)
    : m_Type(Type::Integer),
      m_IsUnsetObject(false),
      m_Data(static_cast<int64_t>(value)) {}

Any::Any(const unsigned long &value)
    : m_Type(Type::Integer),
      m_IsUnsetObject(false),
      m_Data(static_cast<int64_t>(value)) {}

Any::Any(const unsigned long long &value)
    : m_Type(Type::Integer),
      m_IsUnsetObject(false),
      m_Data(static_cast<int64_t>(value)) {}

Any::Any(const float &value)
    : m_Type(Type::Float),
      m_IsUnsetObject(false),
      m_Data(static_cast<double>(value)) {}

Any::Any(const double &value)
    : m_Type(Type::Float),
      m_IsUnsetObject(false),
      m_Data(value) {}

Any::Any(const String &value)
    : m_Type(Type::String),
      m_IsUnsetObject(false),
      m_Data(new String(value)) {
    _validate();
}

Any::Any(const char *value)
    : m_Type(Type::String),
      m_IsUnsetObject(false),
      m_Data(new String(value)) {
    _validate();
}

Any::Any(const bool &value)
    : m_Type(Type::Boolean),
      m_IsUnsetObject(false),
      m_Data(value) {}

Any::Any(const Array &value)
    : m_Type(Type::Array),
      m_IsUnsetObject(false),
      m_Data(value._clone()) {
    _validate();
}

/**--- Any Conversion Operator ---**/

Any::operator char() const {
    switch (m_Type) {
        case Type::String: {
            if (!m_IsUnsetObject && m_Data.string->length() == 1) {
                return m_Data.string->charAt(0);
            } else {
                return 0;
            }
        }
        case Type::Boolean: {
            return m_Data.boolean ? 1 : 0;
        }
        case Type::Integer: {
            return static_cast<char>(m_Data.integer);
        }
        case Type::Float: {
            return static_cast<char>(m_Data.floating);
        }
        default: {
            return 0;
        }
    }
}

Any::operator signed char() const {
    return operator char();
}

Any::operator short() const {
    return operator long();
}

Any::operator int() const {
    return operator long();
}

Any::operator long() const {
    switch (m_Type) {
        case Type::String: {
            if (!m_IsUnsetObject) {
                return m_Data.string->toInt();
            } else {
                return 0;
            }
        }
        case Type::Boolean: {
            return m_Data.boolean ? 1 : 0;
        }
        case Type::Integer: {
            return static_cast<long>(m_Data.integer);
        }
        case Type::Float: {
            return static_cast<long>(m_Data.floating);
        }
        default: {
            return 0;
        }
    }
}

Any::operator long long() const {
    switch (m_Type) {
        case Type::String: {
            return AnyParser::parseInt(*m_Data.string);
        }
        case Type::Boolean: {
            return m_Data.boolean ? 1 : 0;
        }
        case Type::Integer: {
            return static_cast<long long>(m_Data.integer);
        }
        case Type::Float: {
            return static_cast<long long>(m_Data.floating);
        }
        default: {
            return 0;
        }
    }
}

Any::operator unsigned char() const {
    return operator char();
}

Any::operator unsigned short() const {
    return operator long();
}

Any::operator unsigned int() const {
    return operator long();
}

Any::operator unsigned long() const {
    return operator long();
}

Any::operator unsigned long long() const {
    return operator long long();
}

Any::operator float() const {
    return operator double();
}

Any::operator double() const {
    switch (m_Type) {
        case Type::String: {
            return (*m_Data.string).toDouble();
        }
        case Type::Boolean: {
            return m_Data.boolean ? 1 : 0;
        }
        case Type::Integer: {
            return static_cast<double>(m_Data.integer);
        }
        case Type::Float: {
            return m_Data.floating;
        }
        default: {
            return 0;
        }
    }
}

Any::operator bool() const {
    switch (m_Type) {
        case Type::Object:
            return *m_Data.object;
        case Type::Array:
            return *m_Data.array;
        case Type::String:
            return m_Data.string->isEmpty();
        case Type::Integer:
            return m_Data.integer;
        case Type::Float:
            return m_Data.floating;
        case Type::Boolean:
            return m_Data.boolean;
        default:
            return false;
    }
}

Any::operator String() const {
    switch (m_Type) {
        case Type::Object:
            return m_Data.object->toString();
        case Type::Array:
            return m_Data.array->toString();
        case Type::String:
            return *m_Data.string;
        case Type::Integer:
            return AnyParser::toString(m_Data.integer);
        case Type::Float:
            return AnyParser::toString(m_Data.floating);
        case Type::Boolean:
            return m_Data.boolean ? AnyParser::TRUE : AnyParser::FALSE;
        default:
            return AnyParser::NULL_;
    }
}

Any::operator Array &() const {
    if (m_Type != Type::Array) {
        _release();
        m_Type       = Type::Array;
        m_Data.array = new Array();
        _validate();
    }
    return *m_Data.array;
}

/**--- Any Copy Assignment Operator ---**/

Any &Any::operator=(const Any &e) {
    if (this == &e) {
        return *this;
    }
    _release();

    m_Type          = e.m_Type;
    m_IsUnsetObject = e.m_IsUnsetObject;

    if (m_Type == Type::Null) {
        return *this;
    }

    switch (m_Type) {
        case Type::Object:
            m_Data.object = e.m_Data.object->clone();
            break;
        case Type::Array:
            m_Data.array = e.m_Data.array->_clone();
            break;
        case Type::String:
            m_Data.string = new String(*e.m_Data.string);
            break;
        case Type::Integer:
            m_Data.integer = e.m_Data.integer;
            break;
        case Type::Float:
            m_Data.floating = e.m_Data.floating;
            break;
        case Type::Boolean:
            m_Data.boolean = e.m_Data.boolean;
            break;
    }

    _validate();
    return *this;
}

/**--- Any Move Assignment Operator ---**/

Any &Any::operator=(Any &&e) {
    if (this == &e) {
        return *this;
    }
    _release();

    m_Type          = e.m_Type;
    m_IsUnsetObject = e.m_IsUnsetObject;
    m_Data          = e.m_Data;
    e.m_Type        = Type::Null;
    e.m_Data.string = NULL;

    _validate();
    return *this;
}

/**--- Any Square Bracket Operator ---**/

Any &Any::operator[](const Any &index) {
    return operator[](index.as<unsigned long>());
}

Any &Any::operator[](const char &index) {
    return operator[](static_cast<unsigned long>(index));
}

Any &Any::operator[](const signed char &index) {
    return operator[](static_cast<unsigned long>(index));
}

Any &Any::operator[](const short &index) {
    return operator[](static_cast<unsigned long>(index));
}

/**
 * @brief Get an element from the Array.
 * The caller is responsible for ensuring that the value
 * is an Array and the index is within bounds.
 * If the value is not an Array or the index is out of bounds,
 * the behavior is undefined.
 *
 * @param index is the index of the element.
 * @return A reference to the element.
 */
Any &Any::operator[](const int &index) {
    return operator[](static_cast<unsigned long>(index));
}

Any &Any::operator[](const long &index) {
    return operator[](static_cast<unsigned long>(index));
}

Any &Any::operator[](const unsigned char &index) {
    return operator[](static_cast<unsigned long>(index));
}

Any &Any::operator[](const unsigned short &index) {
    return operator[](static_cast<unsigned long>(index));
}

Any &Any::operator[](const unsigned int &index) {
    return operator[](static_cast<unsigned long>(index));
}

Any &Any::operator[](const unsigned long &index) {
    return m_Data.array->get(index);
}

const Any &Any::operator[](const Any &index) const {
    return operator[](index.as<unsigned long>());
}

const Any &Any::operator[](const char &index) const {
    return operator[](static_cast<unsigned long>(index));
}

const Any &Any::operator[](const signed char &index) const {
    return operator[](static_cast<unsigned long>(index));
}

const Any &Any::operator[](const short &index) const {
    return operator[](static_cast<unsigned long>(index));
}

/**
 * @brief Get an element from the Array.
 * The caller is responsible for ensuring that the value
 * is an Array and the index is within bounds.
 * If the value is not an Array or the index is out of bounds,
 * the behavior is undefined.
 *
 * @param index is the index of the element.
 * @return A reference to the element.
 */
const Any &Any::operator[](const int &index) const {
    return operator[](static_cast<unsigned long>(index));
}

const Any &Any::operator[](const long &index) const {
    return operator[](static_cast<unsigned long>(index));
}

const Any &Any::operator[](const unsigned char &index) const {
    return operator[](static_cast<unsigned long>(index));
}

const Any &Any::operator[](const unsigned short &index) const {
    return operator[](static_cast<unsigned long>(index));
}

const Any &Any::operator[](const unsigned int &index) const {
    return operator[](static_cast<unsigned long>(index));
}

const Any &Any::operator[](const unsigned long &index) const {
    return m_Data.array->get(index);
}

/**--- Any Comparison Equal Operator ---**/

bool Any::operator==(const Any &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const char &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const signed char &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const short &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const int &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const long &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const long long &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const unsigned char &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const unsigned short &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const unsigned int &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const unsigned long &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const unsigned long long &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const float &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const double &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const bool &e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const char *e) const {
    return _compareTo(e) == 0;
}

bool Any::operator==(const String &e) const {
    return _compareTo(e) == 0;
}

/**--- Any Comparison Not Equal Operator ---**/

bool Any::operator!=(const Any &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const char &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const signed char &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const short &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const int &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const long &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const long long &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const unsigned char &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const unsigned short &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const unsigned int &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const unsigned long &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const unsigned long long &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const float &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const double &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const bool &e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const char *e) const {
    return _compareTo(e) != 0;
}

bool Any::operator!=(const String &e) const {
    return _compareTo(e) != 0;
}

/**--- Any Comparison Less Than Operator ---**/

bool Any::operator<(const Any &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const char &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const signed char &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const short &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const int &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const long &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const long long &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const unsigned char &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const unsigned short &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const unsigned int &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const unsigned long &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const unsigned long long &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const float &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const double &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const bool &e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const char *e) const {
    return _compareTo(e) == -1;
}

bool Any::operator<(const String &e) const {
    return _compareTo(e) == -1;
}

/**--- Any Comparison Larger Than Operator ---**/

bool Any::operator>(const Any &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const char &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const signed char &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const short &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const int &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const long &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const long long &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const unsigned char &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const unsigned short &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const unsigned int &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const unsigned long &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const unsigned long long &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const float &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const double &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const bool &e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const char *e) const {
    return _compareTo(e) == 1;
}

bool Any::operator>(const String &e) const {
    return _compareTo(e) == 1;
}

/**--- Any Comparison Less Than or Equal Operator ---**/

bool Any::operator<=(const Any &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const char &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const signed char &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const short &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const int &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const long &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const long long &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const unsigned char &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const unsigned short &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const unsigned int &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const unsigned long &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const unsigned long long &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const float &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const double &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const bool &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const char *e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

bool Any::operator<=(const String &e) const {
    int result = _compareTo(e);
    return result <= 0 && result != -2;
}

/**--- Any Comparison Larger Than or Equal Operator ---**/

bool Any::operator>=(const Any &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const char &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const signed char &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const short &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const int &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const long &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const long long &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const unsigned char &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const unsigned short &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const unsigned int &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const unsigned long &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const unsigned long long &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const float &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const double &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const bool &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const char *e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

bool Any::operator>=(const String &e) const {
    int result = _compareTo(e);
    return result >= 0 && result != -2;
}

/**--- Any Addition Operator ---**/

Any Any::operator+(const Any &e) const {
    switch (m_Type) {
        case Type::Integer: {
            switch (e.m_Type) {
                case Type::Integer:
                    return m_Data.integer + e.m_Data.integer;
                case Type::Float:
                    return m_Data.integer + e.m_Data.floating;
                case Type::String:
                    if (!e.m_IsUnsetObject) {
                        return as<String>() + e.as<String>();
                    } else {
                        return *this;
                    }
                default:
                    return *this;
            }
        }
        case Type::Float: {
            switch (e.m_Type) {
                case Type::Integer:
                    return m_Data.floating + e.m_Data.integer;
                case Type::Float:
                    return m_Data.floating + e.m_Data.floating;
                case Type::String:
                    if (!e.m_IsUnsetObject) {
                        return as<String>() + e.as<String>();
                    } else {
                        return *this;
                    }
                default:
                    return *this;
            }
        }
        case Type::String: {
            switch (e.m_Type) {
                case Type::String:
                    if (!e.m_IsUnsetObject) {
                        return as<String>() + e.as<String>();
                    } else {
                        return *this;
                    }
                case Type::Integer:
                case Type::Float:
                case Type::Boolean:
                    return as<String>() + e.as<String>();
                default:
                    return *this;
            }
        }
        case Type::Boolean: {
            switch (e.m_Type) {
                case Type::String:
                    if (!e.m_IsUnsetObject) {
                        return as<String>() + e.as<String>();
                    } else {
                        return *this;
                    }
                default:
                    return *this;
            }
        }
        default: {
            return *this;
        }
    }
}

Any Any::operator+(const char &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const signed char &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const short &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const int &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const long &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const long long &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const unsigned char &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const unsigned short &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const unsigned int &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const unsigned long &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const unsigned long long &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const float &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const double &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const bool &e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const char *e) const {
    return Any::operator+(Any(e));
}

Any Any::operator+(const String &e) const {
    return Any::operator+(Any(e));
}

/**--- Any Substraction Operator ---**/

Any Any::operator-(const Any &e) const {
    switch (m_Type) {
        case Type::Integer: {
            switch (e.m_Type) {
                case Type::Integer:
                    return m_Data.integer - e.m_Data.integer;
                case Type::Float:
                    return m_Data.integer - e.m_Data.floating;
                default:
                    return *this;
            }
        }
        case Type::Float: {
            switch (e.m_Type) {
                case Type::Integer:
                    return m_Data.floating - e.m_Data.integer;
                case Type::Float:
                    return m_Data.floating - e.m_Data.floating;
                default:
                    return *this;
            }
        }
    }
    return *this;
}

Any Any::operator-(const char &e) const {
    return Any::operator-(Any(e));
}

Any Any::operator-(const signed char &e) const {
    return Any::operator-(Any(e));
}

Any Any::operator-(const short &e) const {
    return Any::operator-(Any(e));
}

Any Any::operator-(const int &e) const {
    return Any::operator-(Any(e));
}

Any Any::operator-(const long &e) const {
    return Any::operator-(Any(e));
}

Any Any::operator-(const long long &e) const {
    return Any::operator-(Any(e));
}

Any Any::operator-(const unsigned char &e) const {
    return Any::operator-(Any(e));
}

Any Any::operator-(const unsigned short &e) const {
    return Any::operator-(Any(e));
}

Any Any::operator-(const unsigned int &e) const {
    return Any::operator-(Any(e));
}

Any Any::operator-(const unsigned long &e) const {
    return Any::operator-(Any(e));
}

Any Any::operator-(const unsigned long long &e) const {
    return Any::operator-(Any(e));
}

Any Any::operator-(const float &e) const {
    return Any::operator-(Any(e));
}

Any Any::operator-(const double &e) const {
    return Any::operator-(Any(e));
}

/**--- Any Multiplication Operator ---**/

Any Any::operator*(const Any &e) const {
    switch (m_Type) {
        case Type::Integer: {
            switch (e.m_Type) {
                case Type::Integer:
                    return m_Data.integer * e.m_Data.integer;
                case Type::Float:
                    return m_Data.integer * e.m_Data.floating;
                default:
                    return *this;
            }
        }
        case Type::Float: {
            switch (e.m_Type) {
                case Type::Integer:
                    return m_Data.floating * e.m_Data.integer;
                case Type::Float:
                    return m_Data.floating * e.m_Data.floating;
                default:
                    return *this;
            }
        }
    }
    return *this;
}

Any Any::operator*(const char &e) const {
    return Any::operator*(Any(e));
}

Any Any::operator*(const signed char &e) const {
    return Any::operator*(Any(e));
}

Any Any::operator*(const short &e) const {
    return Any::operator*(Any(e));
}

Any Any::operator*(const int &e) const {
    return Any::operator*(Any(e));
}

Any Any::operator*(const long &e) const {
    return Any::operator*(Any(e));
}

Any Any::operator*(const long long &e) const {
    return Any::operator*(Any(e));
}

Any Any::operator*(const unsigned char &e) const {
    return Any::operator*(Any(e));
}

Any Any::operator*(const unsigned short &e) const {
    return Any::operator*(Any(e));
}

Any Any::operator*(const unsigned int &e) const {
    return Any::operator*(Any(e));
}

Any Any::operator*(const unsigned long &e) const {
    return Any::operator*(Any(e));
}

Any Any::operator*(const unsigned long long &e) const {
    return Any::operator*(Any(e));
}

Any Any::operator*(const float &e) const {
    return Any::operator*(Any(e));
}

Any Any::operator*(const double &e) const {
    return Any::operator*(Any(e));
}

/**--- Any Division Operator ---**/

Any Any::operator/(const Any &e) const {
    switch (m_Type) {
        case Type::Integer: {
            switch (e.m_Type) {
                case Type::Integer:
                    return m_Data.integer / e.m_Data.integer;
                case Type::Float:
                    return m_Data.integer / e.m_Data.floating;
                default:
                    return *this;
            }
        }
        case Type::Float: {
            switch (e.m_Type) {
                case Type::Integer:
                    return m_Data.floating / e.m_Data.integer;
                case Type::Float:
                    return m_Data.floating / e.m_Data.floating;
                default:
                    return *this;
            }
        }
    }
    return *this;
}

Any Any::operator/(const char &e) const {
    return Any::operator/(Any(e));
}

Any Any::operator/(const signed char &e) const {
    return Any::operator/(Any(e));
}

Any Any::operator/(const short &e) const {
    return Any::operator/(Any(e));
}

Any Any::operator/(const int &e) const {
    return Any::operator/(Any(e));
}

Any Any::operator/(const long &e) const {
    return Any::operator/(Any(e));
}

Any Any::operator/(const long long &e) const {
    return Any::operator/(Any(e));
}

Any Any::operator/(const unsigned char &e) const {
    return Any::operator/(Any(e));
}

Any Any::operator/(const unsigned short &e) const {
    return Any::operator/(Any(e));
}

Any Any::operator/(const unsigned int &e) const {
    return Any::operator/(Any(e));
}

Any Any::operator/(const unsigned long &e) const {
    return Any::operator/(Any(e));
}

Any Any::operator/(const unsigned long long &e) const {
    return Any::operator/(Any(e));
}

Any Any::operator/(const float &e) const {
    return Any::operator/(Any(e));
}

Any Any::operator/(const double &e) const {
    return Any::operator/(Any(e));
}

/**--- Any Modulo Operator ---**/

Any Any::operator%(const Any &e) const {
    switch (m_Type) {
        case Type::Integer: {
            switch (e.m_Type) {
                case Type::Integer:
                    return m_Data.integer % e.m_Data.integer;
                case Type::Float:
                    return fmod(m_Data.integer, e.m_Data.floating);
                default:
                    return *this;
            }
        }
        case Type::Float: {
            switch (e.m_Type) {
                case Type::Integer:
                    return fmod(m_Data.floating, e.m_Data.integer);
                case Type::Float:
                    return fmod(m_Data.floating, e.m_Data.floating);
                default:
                    return *this;
            }
        }
    }
    return *this;
}

Any Any::operator%(const char &e) const {
    return Any::operator%(Any(e));
}

Any Any::operator%(const signed char &e) const {
    return Any::operator%(Any(e));
}

Any Any::operator%(const short &e) const {
    return Any::operator%(Any(e));
}

Any Any::operator%(const long &e) const {
    return Any::operator%(Any(e));
}

Any Any::operator%(const int &e) const {
    return Any::operator%(Any(e));
}

Any Any::operator%(const long long &e) const {
    return Any::operator%(Any(e));
}

Any Any::operator%(const unsigned char &e) const {
    return Any::operator%(Any(e));
}

Any Any::operator%(const unsigned short &e) const {
    return Any::operator%(Any(e));
}

Any Any::operator%(const unsigned int &e) const {
    return Any::operator%(Any(e));
}

Any Any::operator%(const unsigned long &e) const {
    return Any::operator%(Any(e));
}

Any Any::operator%(const unsigned long long &e) const {
    return Any::operator%(Any(e));
}

Any Any::operator%(const float &e) const {
    return Any::operator%(Any(e));
}

Any Any::operator%(const double &e) const {
    return Any::operator%(Any(e));
}

/**--- Any Addition Assignment Operator ---**/

Any &Any::operator+=(const Any &e) {
    switch (m_Type) {
        case Type::Integer: {
            switch (e.m_Type) {
                case Type::Integer:
                    m_Data.integer += e.m_Data.integer;
                    break;
                case Type::Float:
                    m_Data.floating = m_Data.integer + e.m_Data.floating;
                    m_Type          = Type::Float;
                    break;
                case Type::String:
                    if (!e.m_IsUnsetObject) {
                        m_Data.string = new String(AnyParser::toString(m_Data.integer) + e.as<String>());
                        m_Type        = Type::String;
                        _validate();
                    }
                    break;
            }
            break;
        }
        case Type::Float: {
            switch (e.m_Type) {
                case Type::Integer:
                    m_Data.floating += e.m_Data.integer;
                    break;
                case Type::Float:
                    m_Data.floating += e.m_Data.floating;
                    break;
                case Type::String:
                    if (!e.m_IsUnsetObject) {
                        m_Data.string = new String(AnyParser::toString(m_Data.floating) + e.as<String>());
                        m_Type        = Type::String;
                        _validate();
                    }
                    break;
            }
            break;
        }
        case Type::String: {
            if (!m_IsUnsetObject) {
                switch (e.m_Type) {
                    case Type::Integer:
                        m_Data.string->concat(AnyParser::toString(e.m_Data.integer));
                        break;
                    case Type::Float:
                        m_Data.string->concat(AnyParser::toString(e.m_Data.floating));
                        break;
                    case Type::String:
                        if (!e.m_IsUnsetObject) {
                            m_Data.string->concat(*e.m_Data.string);
                        }
                        break;
                    case Type::Boolean:
                        m_Data.string->concat(e.as<String>());
                        break;
                }
            }
            break;
        }
        case Type::Boolean: {
            if (e.m_Type == Type::String && !e.m_IsUnsetObject) {
                m_Data.string = new String(as<String>() + e.as<String>());
                m_Type        = Type::String;
                _validate();
            }
        }
    }
    return *this;
}

Any &Any::operator+=(const char &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const signed char &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const short &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const int &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const long &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const long long &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const unsigned char &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const unsigned short &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const unsigned int &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const unsigned long &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const unsigned long long &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const float &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const double &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const bool &e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const char *e) {
    return operator+=(Any(e));
}

Any &Any::operator+=(const String &e) {
    return operator+=(Any(e));
}

/**--- Any Substraction Assignment Operator ---**/

Any &Any::operator-=(const Any &e) {
    switch (m_Type) {
        case Type::Integer: {
            switch (e.m_Type) {
                case Type::Integer:
                    m_Data.integer -= e.m_Data.integer;
                    break;
                case Type::Float:
                    m_Data.floating = m_Data.integer - e.m_Data.floating;
                    m_Type          = Type::Float;
                    break;
            }
            break;
        }
        case Type::Float: {
            switch (e.m_Type) {
                case Type::Integer:
                    m_Data.floating -= e.m_Data.integer;
                    break;
                case Type::Float:
                    m_Data.floating -= e.m_Data.floating;
                    break;
            }
            break;
        }
    }
    return *this;
}

Any &Any::operator-=(const char &e) {
    return operator-=(Any(e));
}

Any &Any::operator-=(const signed char &e) {
    return operator-=(Any(e));
}

Any &Any::operator-=(const short &e) {
    return operator-=(Any(e));
}

Any &Any::operator-=(const int &e) {
    return operator-=(Any(e));
}

Any &Any::operator-=(const long &e) {
    return operator-=(Any(e));
}

Any &Any::operator-=(const long long &e) {
    return operator-=(Any(e));
}

Any &Any::operator-=(const unsigned char &e) {
    return operator-=(Any(e));
}

Any &Any::operator-=(const unsigned short &e) {
    return operator-=(Any(e));
}

Any &Any::operator-=(const unsigned int &e) {
    return operator-=(Any(e));
}

Any &Any::operator-=(const unsigned long &e) {
    return operator-=(Any(e));
}

Any &Any::operator-=(const unsigned long long &e) {
    return operator-=(Any(e));
}

Any &Any::operator-=(const float &e) {
    return operator-=(Any(e));
}

Any &Any::operator-=(const double &e) {
    return operator-=(Any(e));
}

/**--- Any Multiplication Assignment Operator ---**/

Any &Any::operator*=(const Any &e) {
    switch (m_Type) {
        case Type::Integer: {
            switch (e.m_Type) {
                case Type::Integer:
                    m_Data.integer *= e.m_Data.integer;
                    break;
                case Type::Float:
                    m_Data.floating = m_Data.integer * e.m_Data.floating;
                    m_Type          = Type::Float;
                    break;
            }
            break;
        }
        case Type::Float: {
            switch (e.m_Type) {
                case Type::Integer:
                    m_Data.floating *= e.m_Data.integer;
                    break;
                case Type::Float:
                    m_Data.floating *= e.m_Data.floating;
                    break;
            }
            break;
        }
    }
    return *this;
}

Any &Any::operator*=(const char &e) {
    return operator*=(Any(e));
}

Any &Any::operator*=(const signed char &e) {
    return operator*=(Any(e));
}

Any &Any::operator*=(const short &e) {
    return operator*=(Any(e));
}

Any &Any::operator*=(const int &e) {
    return operator*=(Any(e));
}

Any &Any::operator*=(const long &e) {
    return operator*=(Any(e));
}

Any &Any::operator*=(const long long &e) {
    return operator*=(Any(e));
}

Any &Any::operator*=(const unsigned char &e) {
    return operator*=(Any(e));
}

Any &Any::operator*=(const unsigned short &e) {
    return operator*=(Any(e));
}

Any &Any::operator*=(const unsigned int &e) {
    return operator*=(Any(e));
}

Any &Any::operator*=(const unsigned long &e) {
    return operator*=(Any(e));
}

Any &Any::operator*=(const unsigned long long &e) {
    return operator*=(Any(e));
}

Any &Any::operator*=(const float &e) {
    return operator*=(Any(e));
}

Any &Any::operator*=(const double &e) {
    return operator*=(Any(e));
}

/**--- Any Division Assignment Operator ---**/

Any &Any::operator/=(const Any &e) {
    switch (m_Type) {
        case Type::Integer: {
            switch (e.m_Type) {
                case Type::Integer:
                    m_Data.integer /= e.m_Data.integer;
                    break;
                case Type::Float:
                    m_Data.floating = m_Data.integer / e.m_Data.floating;
                    m_Type          = Type::Float;
                    break;
            }
            break;
        }
        case Type::Float: {
            switch (e.m_Type) {
                case Type::Integer:
                    m_Data.floating /= e.m_Data.integer;
                    break;
                case Type::Float:
                    m_Data.floating /= e.m_Data.floating;
                    break;
            }
            break;
        }
    }
    return *this;
}

Any &Any::operator/=(const char &e) {
    return operator/=(Any(e));
}

Any &Any::operator/=(const signed char &e) {
    return operator/=(Any(e));
}

Any &Any::operator/=(const short &e) {
    return operator/=(Any(e));
}

Any &Any::operator/=(const int &e) {
    return operator/=(Any(e));
}

Any &Any::operator/=(const long &e) {
    return operator/=(Any(e));
}

Any &Any::operator/=(const long long &e) {
    return operator/=(Any(e));
}

Any &Any::operator/=(const unsigned char &e) {
    return operator/=(Any(e));
}

Any &Any::operator/=(const unsigned short &e) {
    return operator/=(Any(e));
}

Any &Any::operator/=(const unsigned int &e) {
    return operator/=(Any(e));
}

Any &Any::operator/=(const unsigned long &e) {
    return operator/=(Any(e));
}

Any &Any::operator/=(const unsigned long long &e) {
    return operator/=(Any(e));
}

Any &Any::operator/=(const float &e) {
    return operator/=(Any(e));
}

Any &Any::operator/=(const double &e) {
    return operator/=(Any(e));
}

/**--- Any Modulo Assignment Operator ---**/

Any &Any::operator%=(const Any &e) {
    switch (m_Type) {
        case Type::Integer: {
            switch (e.m_Type) {
                case Type::Integer:
                    m_Data.integer %= e.m_Data.integer;
                    break;
                case Type::Float:
                    m_Data.floating = fmod(m_Data.integer, e.m_Data.floating);
                    m_Type          = Type::Float;
                    break;
            }
            break;
        }
        case Type::Float: {
            switch (e.m_Type) {
                case Type::Integer:
                    m_Data.floating = fmod(m_Data.floating, e.m_Data.integer);
                    break;
                case Type::Float:
                    m_Data.floating = fmod(m_Data.floating, e.m_Data.floating);
                    break;
            }
            break;
        }
    }
    return *this;
}

Any &Any::operator%=(const char &e) {
    return operator%=(Any(e));
}

Any &Any::operator%=(const signed char &e) {
    return operator%=(Any(e));
}

Any &Any::operator%=(const short &e) {
    return operator%=(Any(e));
}

Any &Any::operator%=(const int &e) {
    return operator%=(Any(e));
}

Any &Any::operator%=(const long &e) {
    return operator%=(Any(e));
}

Any &Any::operator%=(const long long &e) {
    return operator%=(Any(e));
}

Any &Any::operator%=(const unsigned char &e) {
    return operator%=(Any(e));
}

Any &Any::operator%=(const unsigned short &e) {
    return operator%=(Any(e));
}

Any &Any::operator%=(const unsigned int &e) {
    return operator%=(Any(e));
}

Any &Any::operator%=(const unsigned long &e) {
    return operator%=(Any(e));
}

Any &Any::operator%=(const unsigned long long &e) {
    return operator%=(Any(e));
}

Any &Any::operator%=(const float &e) {
    return operator%=(Any(e));
}

Any &Any::operator%=(const double &e) {
    return operator%=(Any(e));
}

/**--- Any Prefix Increment Operator ---**/

Any &Any::operator++() {
    switch (m_Type) {
        case Type::Integer:
            m_Data.integer++;
            break;
        case Type::Float:
            m_Data.floating++;
            break;
    }
    return *this;
}

/**--- Any Postfix Increment Operator ---**/

Any Any::operator++(int) {
    Any e = *this;
    switch (m_Type) {
        case Type::Integer:
            m_Data.integer++;
            break;
        case Type::Float:
            m_Data.floating++;
            break;
    }
    return e;
}

/**--- Any Prefix decrement Operator ---**/

Any &Any::operator--() {
    switch (m_Type) {
        case Type::Integer:
            m_Data.integer--;
            break;
        case Type::Float:
            m_Data.floating--;
            break;
    }
    return *this;
}

/**--- Any Postfix decrement Operator ---**/

Any Any::operator--(int) {
    Any e = *this;
    switch (m_Type) {
        case Type::Integer:
            m_Data.integer--;
            break;
        case Type::Float:
            m_Data.floating--;
            break;
    }
    return e;
}

/**--- Any Explicit Conversion ---**/

/**
 * @brief Explicitly convert to a 64-bit Integer value.
 * If the value is an Object or Array, the conversion will fail, and zero will be returned.
 * If the value is a String, the conversion will attempt to convert the string to an integer.
 * If the value is a Boolean, the conversion will return 1 for true and 0 for false.
 * If the value is Null, the conversion will return 0.
 *
 * @return The Integer representation of this value.
 */
int64_t Any::toInt() const {
    return as<int64_t>();
}

/**
 * @brief Explicitly convert to a Float value.
 * If the value is an Object or Array, the conversion will fail, and zero will be returned.
 * If the value is a String, the conversion will attempt to convert the string to an integer.
 * If the value is a Boolean, the conversion will return 1 for true and 0 for false.
 * If the value is Null, the conversion will return 0.
 *
 * @return The Float representation of this value.
 */
float Any::toFloat() const {
    return as<float>();
}

/**
 * @brief Explicitly convert to a Double value.
 * If the value is an Object or Array, the conversion will fail, and zero will be returned.
 * If the value is a String, the conversion will attempt to convert the string to an integer.
 * If the value is a Boolean, the conversion will return 1 for true and 0 for false.
 * If the value is Null, the conversion will return 0.
 *
 * @return The Double representation of this value.
 */
double Any::toDouble() const {
    return as<double>();
}

/**
 * @brief Explicitly convert to a Boolean value.
 * If the value is an Object, the returned value is depend on the operator bool()
 * of the object, the operator bool() of the object can be overridden by its inheriting class,
 * by default the operator bool() of the object returns true if the vector is not empty, false otherwise.
 * If the value is an Array or a String, the returned value is depend on the size of the array or string,
 * the returned value is true if the size is not zero, otherwise false.
 * If the value is a Number, the returned value is depend on the value of the number,
 * the returned value is true if the value is not zero, otherwise false.
 *
 * @return The Boolean representation of this value.
 */
bool Any::toBool() const {
    return as<boolean>();
}

/**
 * @brief Explicitly convert to a String value.
 * If the value is an Object or Array, this method will call the toString() method of the object or array.
 * If the value is a Number, the conversion will attempt to convert the number into a String.
 * If the value is a Boolean, the conversion will return "true" for true and "false" for false.
 * If the value is Null, the conversion will return "null".
 *
 * @return The String representation of this value.
 */
String Any::toString() const {
    return as<String>();
}

/**
 * @brief Explicitly convert to a const char* value.
 * If the value is a String, the conversion will return the c_str() of the string.
 * If the value is not a String, the conversion will return NULL.
 *
 * @return A pointer to the string data.
 */
const char *Any::c_str() const {
    if (m_Type == Type::String && !m_IsUnsetObject) {
        return m_Data.string->c_str();
    }
    return NULL;
}

/**
 * @brief Serialize the value into a String.
 * If the value is Object or Array, this method will call the serialize() method of the object or array.
 * If the value is a String, this method will escape the string and add quotes to it.
 * If the value is a Number, the conversion will attempt to convert the number into a String.
 * If the value is a Boolean, the conversion will return "true" for true and "false" for false.
 * If the value is Null, the conversion will return "null".
 *
 * @return The serialized String representation of this value.
 */
String Any::serialize() const {
    switch (m_Type) {
        case Type::Object:
            return m_Data.object->serialize();
        case Type::Array:
            return m_Data.array->serialize();
        case Type::String:
            if (m_IsUnsetObject) {
                return *m_Data.string;
            } else {
                return AnyParser::serialize(*m_Data.string);
            }
        case Type::Integer:
        case Type::Float:
        case Type::Boolean:
            return as<String>();
        default:
            return AnyParser::NULL_;
    }
}

/**--- Any Miscellaneous ---**/

/**
 * @brief Get the type of the value.
 *
 * @return The type of the value. Possible values are: Object, Array, String, Integer, Float, Boolean, Null.
 */
Any::Type Any::getType() const {
    if (m_Type == Type::String && m_IsUnsetObject) {
        return Type::Object;
    }

    return m_Type;
}

/**
 * @brief Get the type name of the value.
 *
 * @return The type name of the value. Possible values are: "object", "array", "string", "integer", "float", "boolean",
 * "null".
 */
String Any::getTypeName() const {
    if (m_Type == Type::String && m_IsUnsetObject) {
        return "object";
    }

    switch (m_Type) {
        case Any::Type::Object:
            return "object";
        case Any::Type::Array:
            return "array";
        case Any::Type::String:
            return "string";
        case Any::Type::Integer:
            return "integer";
        case Any::Type::Float:
            return "float";
        case Any::Type::Boolean:
            return "boolean";
        case Any::Type::Null:
            return "null";
        default:
            return "undefined";
    }
}

/**
 * @brief Get the size of the value.
 * If the value is an Object or Array, the size is the number of elements in the object or array.
 * If the value is a String, the size is the length of the string.
 * If the value is not an Object, Array or String, the size is zero.
 *
 * @return The size of the value.
 */
size_t Any::size() const {
    switch (m_Type) {
        case Type::Object:
            return m_Data.object->size();
        case Type::Array:
            return m_Data.array->size();
        case Type::String:
            if (!m_IsUnsetObject) {
                return m_Data.string->length();
            } else {
                return 0;
            }
        default:
            return 0;
    }
}

/**
 * @brief Get the last index of the value.
 * If the value is an Object, Array, or String, the last index is (size - 1).
 * Otherwise, the last index is -1.
 *
 * @return The last index of the value.
 */
size_t Any::lastIndex() const {
    return size() - 1;
}

/**
 * @brief Remove the element at the specified index.
 * If the value is an Array or String, the element at the specified index will be removed.
 * Otherwise, this method does nothing.
 *
 * @param index The index of the first element to remove.
 * @param count The number of elements to be removed.
 */
void Any::remove(const size_t &index, const size_t &count) {
    if (m_Type == Type::String && !m_IsUnsetObject) {
        m_Data.string->remove(index, count);
    } else if (m_Type == Type::Array) {
        m_Data.array->remove(index, count);
    }
}

/**
 * @brief Remove all of the elements from the value.
 * If the value is an Array or String, all of the elements will be removed.
 * Otherwise, this method does nothing.
 */
void Any::clear() {
    switch (m_Type) {
        case Type::Array:
            m_Data.array->clear();
            break;
        case Type::String:
            if (!m_IsUnsetObject) {
                m_Data.string->clear();
            }
            break;
    }
}

/**
 * @brief Check if the value contains the specified element.
 * If the value is an Array, this method will call the contains() method of the array.
 * If the value is a String, this method will check if the string contains the specified element.
 * Otherwise, this method will return false.
 *
 * @param e an element to be checked.
 * @return true if the value contains the specified element. false otherwise.
 */
bool Any::contains(const Any &e) const {
    switch (m_Type) {
        case Type::Array:
            return m_Data.array->contains(e);
        case Type::String:
            if (!m_IsUnsetObject) {
                return m_Data.string->indexOf(e.toString()) != -1;
            } else {
                return false;
            }
        default:
            return false;
    }
}

/**
 * @brief Check if the value is null.
 *
 * @return true if the value is null. false otherwise.
 */
bool Any::isNull() const {
    return m_Type == Type::Null;
}

/**
 * @brief Check if the value is not null.
 *
 * @return true if the value is not null. false otherwise.
 */
bool Any::isNotNull() const {
    return m_Type != Type::Null;
}

/**
 * @brief Check if the value is empty.
 * If the value is an Array, this method will call the isEmpty() method of the array.
 * If the value is a String, this method will check if the string is empty.
 * Otherwise, this method will return true.
 *
 * @return true if the value is empty. false otherwise.
 */
bool Any::isEmpty() const {
    switch (m_Type) {
        case Type::Array:
            m_Data.array->isEmpty();
        case Type::String:
            if (!m_IsUnsetObject) {
                return m_Data.string->isEmpty();
            } else {
                return true;
            }
        default:
            return true;
    }
}

/**
 * @brief Check if the value is not empty.
 * If the value is an Array, this method will call the isNotEmpty() method of the array.
 * If the value is a String, this method will check if the string is not empty.
 * Otherwise, this method will return false.
 *
 * @return true if the value is not empty. false otherwise.
 */
bool Any::isNotEmpty() const {
    return !isEmpty();
}

/**
 * @brief Check if the value is a negative number.
 * If the value is not a number, this method will return false.
 *
 * @return true if the value is a negative number. false otherwise.
 */
bool Any::isNegative() const {
    switch (m_Type) {
        case Type::Integer:
            return static_cast<int64_t>(m_Data.integer) < 0;
        case Type::Float:
            return m_Data.floating < 0;
        default:
            return false;
    }
}

/**
 * @brief Check if the value is an Object.
 *
 * @return true if the value is an Object. false otherwise.
 */
bool Any::isObject() const {
    return m_Type == Type::Object || (m_Type == Type::String && m_IsUnsetObject);
}

/**
 * @brief Check if the value is an Array.
 *
 * @return true if the value is an Array. false otherwise.
 */
bool Any::isArray() const {
    return m_Type == Type::Array;
}

/**
 * @brief Check if the value is a String.
 *
 * @return true if the value is a String. false otherwise.
 */
bool Any::isString() const {
    return m_Type == Type::String && !m_IsUnsetObject;
}

/**
 * @brief Check if the value is a number.
 *
 * @return true if the value is a number. false otherwise.
 */
bool Any::isNumber() const {
    return m_Type == Type::Integer || m_Type == Type::Float;
}

/**
 * @brief Check if the value is a greater than 32-bit integer.
 *
 * @return true if the value is a greater than 32-bit integer. false otherwise.
 */
bool Any::isBigNumber() const {
    if (m_Type == Type::Integer || m_Type == Type::Float) {
        return isNegative() ? -as<double>() > INT32_MAX : as<double>() > UINT32_MAX;
    }
    return false;
}

/**
 * @brief Check if the value is an integer.
 *
 * @return true if the value is an integer. false otherwise.
 */
bool Any::isBool() const {
    return m_Type == Type::Boolean;
}

/**
 * @brief Return an iterator to the beginning of the array.
 * If this object is not an array, the returned iterator is invalid.
 *
 * @return std::vector<Any>::iterator
 */
std::vector<Any>::iterator Any::begin() {
    if (m_Type == Type::Array) {
        return m_Data.array->begin();
    }
    return std::vector<Any>::iterator();
}

/**
 * @brief Return an iterator to the end of the array.
 * If this object is not an array, the returned iterator is invalid.
 *
 * @return std::vector<Any>::iterator
 */
std::vector<Any>::iterator Any::end() {
    if (m_Type == Type::Array) {
        return m_Data.array->end();
    }
    return std::vector<Any>::iterator();
}

/**
 * @brief Parse the given String into an Any object.
 * If the String starts with '{' and ends with '}' it will be parsed as an Object.
 * If the String starts with '[' and ends with ']' it will be parsed as an Array.
 * If the String starts with '"' and ends with '"' it will be parsed as a String.
 * If the String is a number it will be parsed as a Number.
 * If the String is 'true' or 'false' it will be parsed as a Boolean.
 * Otherwise it will be parsed as a Null.
 *
 * NOTE: If the String is an Object, the returned Any object will be a String.
 * The String will be deserialized when the object is accessed. The reasoning behind
 * this is that at the time of parsing, the type of the Object is not known.
 *
 * @param str is the String to parse.
 * @return The parsed Any object.
 */
Any Any::parse(const String &str) {
    if (AnyParser::isObject(str)) {
        Any any(str);
        any.m_IsUnsetObject = true;
        return any;
    }

    if (AnyParser::isString(str)) {
        return str.substring(1, str.length() - 1);
    }

    if (AnyParser::isArray(str)) {
        Array array;
        array.m_Data = AnyParser::parse(str);
        return array;
    }

    return AnyParser::parseLiteral(str);
}

/**
 * @brief Print this object to the given Print object.
 * This method is used by the Arduino Print class.
 * The output is the same as the output of the toString() method.
 *
 * @param p
 * @return size_t
 */
size_t Any::printTo(Print &p) const {
    return p.print(as<String>());
}

/**
 * @brief Delete the data this object contains and set the type to Null.
 *
 */
void Any::_release() const {
    if (m_Type == Type::Object) {
        delete m_Data.object;
    } else if (m_Type == Type::Array) {
        delete m_Data.array;
    } else if (m_Type == Type::String) {
        delete m_Data.string;
    }

    m_Data.integer = 0;
    m_Type         = Type::Null;
}

/**
 * @brief Validate the type and the data this object contains.
 * If the type is Object, Array, or String, but the data is NULL, then change the type to Null.
 */
void Any::_validate() const {
    if ((m_Type == Type::Object || m_Type == Type::Array || m_Type == Type::String) && m_Data.string == NULL) {
        m_Type = Type::Null;
    }
}

/**
 * @brief Compare two Any objects.
 * If both are Objects or Arrays, only compare their equality.
 * If both are Strings, compare their equality first, if not equal, compare their lexicographical order.
 * If both are Numbers, compare their equality first, if not equal, compare their numerical order.
 * If both are Booleans, compare their equality first, if not equal, compare their numerical order.
 * If both are Null, they are equal.
 * If those conditions are not met, then they are not comparable.
 *
 * @param other the other Any object.
 * @return 0 if equal, 1 if greater than other, -1 if less than other, -2 if not comparable
 * (except if both are objects or arrays and both are not equal).
 */
int Any::_compareTo(const Any &other) const {
    if (m_Type == Type::Object && other.m_Type == Type::Object) {
        return m_Data.object->equals(*other.m_Data.object) ? 0 : -2;
    }

    if (m_IsUnsetObject || other.m_IsUnsetObject) {
        if (m_Type == Type::String && m_IsUnsetObject && other.m_Type == Type::String && other.m_IsUnsetObject) {
            return m_Data.string->equals(*other.m_Data.string) ? 0 : -2;
        }

        if (m_Type == Type::String && m_IsUnsetObject && other.m_Type == Type::Object) {
            return m_Data.string->equals(other.m_Data.object->serialize()) ? 0 : -2;
        }

        if (m_Type == Type::Object && other.m_Type == Type::String && other.m_IsUnsetObject) {
            return m_Data.object->serialize().equals(*other.m_Data.string) ? 0 : -2;
        }

        return -2;
    }

    if (m_Type == Type::Array && other.m_Type == Type::Array) {
        return m_Data.array->equals(*other.m_Data.array) ? 0 : -2;
    }

    if (m_Type == Type::String && other.m_Type == Type::String) {
        if (m_Data.string->equals(*other.m_Data.string)) {
            return 0;
        } else {
            return m_Data.string->compareTo(*other.m_Data.string) > 0 ? 1 : -1;
        }
    }

    if (m_Type == Type::Integer && other.m_Type == Type::Integer) {
        if (m_Data.integer == other.m_Data.integer) {
            return 0;
        } else {
            return m_Data.integer > other.m_Data.integer ? 1 : -1;
        }
    }

    if (m_Type == Type::Integer && other.m_Type == Type::Float) {
        if (m_Data.integer == other.m_Data.floating) {
            return 0;
        } else {
            return m_Data.integer > other.m_Data.floating ? 1 : -1;
        }
    }

    if (m_Type == Type::Float && other.m_Type == Type::Integer) {
        if (m_Data.floating == other.m_Data.integer) {
            return 0;
        } else {
            return m_Data.floating > other.m_Data.integer ? 1 : -1;
        }
    }

    if (m_Type == Type::Float && other.m_Type == Type::Float) {
        if (m_Data.floating == other.m_Data.floating) {
            return 0;
        } else {
            return m_Data.floating > other.m_Data.floating ? 1 : -1;
        }
    }

    if (m_Type == Type::Boolean && other.m_Type == Type::Boolean) {
        if (m_Data.boolean == other.m_Data.boolean) {
            return 0;
        } else {
            return m_Data.boolean > other.m_Data.boolean ? 1 : -1;
        }
    }

    if (m_Type == Type::Null && other.m_Type == Type::Null) {
        return 0;
    }

    return -2;
}

/**--- AnyParser ---**/

/**
 * @brief Find the closing Object bracket '}' starting at the given index.
 *
 * @param src The string to search in.
 * @param start The index to start searching at including the opening bracket '{'.
 * @return The index of the closing character, or -1 if not found.
 */
int16_t AnyParser::findClosingObjectBracket(const String &src, const int16_t &start) {
    int16_t count = 0;
    for (int16_t i = start; i < src.length(); i++) {
        if (src[i] == OBJECT_OPEN_BRACKET) {
            count++;
        }

        if (src[i] == OBJECT_CLOSE_BRACKET) {
            count--;
        }

        if (count == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Find the closing Array bracket ']' starting at the given index.
 *
 * @param src The string to search in.
 * @param start The index to start searching at including the opening bracket '['.
 * @return The index of the closing character, or -1 if not found.
 */
int16_t AnyParser::findClosingArrayBracket(const String &src, const int16_t &start) {
    int16_t count = 0;
    for (int16_t i = start; i < src.length(); i++) {
        if (src[i] == ARRAY_OPEN_BRACKET) {
            count++;
        }

        if (src[i] == ARRAY_CLOSE_BRACKET) {
            count--;
        }

        if (count == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Find the closing quote for the given quote character, starting at the
 * given index.
 *
 * @param src The string to search in.
 * @param start The index to start searching at.
 * @return The index of the closing quote, or -1 if not found.
 */
int16_t AnyParser::findClosingQuote(const String &src, const int16_t &start) {
    for (int16_t i = start; i < src.length(); i++) {
        if (src[i] == STRING_BRACKET && src[i - 1] != '\\') {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Convert a float to a string.
 *
 * @param value The float to convert.
 * @return The string representation of the float.
 */
String AnyParser::toString(const float &value) {
    return removeInsignificantZeros(String(value, 5));
}

/**
 * @brief Convert a double to a string.
 *
 * @param value The double to convert.
 * @return The string representation of the double.
 */
String AnyParser::toString(const double &value) {
    return removeInsignificantZeros(String(value, 11));
}

/**
 * @brief Convert an integer to a string.
 *
 * @param value The integer to convert.
 * @return The string representation of the integer.
 */
String AnyParser::toString(const int64_t &value) {
    if (value == 0) {
        return String("0");
    }

    char buf[21];
    uint8_t index = 0;
    int64_t v     = value;

    while (v > 0) {
        buf[index++] = (v % 10) + '0';
        v /= 10;
    }

    String result;
    if (value < 0) {
        result += '-';
    }

    for (uint8_t i = (value < 0 ? 1 : 0); i < index + (value < 0 ? 1 : 0); i++) {
        result += buf[index - i - (value < 0 ? 0 : 1)];
    }

    return result;
}

/**
 * @brief Convert a string to a 64-bit integer.
 *
 * @param str The string to convert.
 * @return The 64-bit integer representation of the string.
 */
int64_t AnyParser::parseInt(const String &str) {
    if (str.isEmpty()) {
        return 0;
    }

    bool isNegative = str[0] == '-';
    uint64_t result = 0;

    for (int i = isNegative || (str[0] == '+') ? 1 : 0; i < str.length(); i++) {
        if (!isdigit(str.charAt(i))) {
            return 0;
        }
        result *= 10;
        result += (str.charAt(i) - '0');
    }

    return isNegative ? -result : result;
}

/**
 * @brief Check if the String is a serialized array.
 * This method only checks if the String starts with '[' and ends with ']'.
 * It does not check if the String is a valid array.
 *
 * @param str a String to check.
 * @return true if the String is a serialized array. false otherwise.
 */
bool AnyParser::isArray(const String &str) {
    if (str.length() < 2) {
        return false;
    }

    return str[0] == ARRAY_OPEN_BRACKET && str[str.length() - 1] == ARRAY_CLOSE_BRACKET;
}

/**
 * @brief Check if the String is a serialized object.
 * This method only checks if the String starts with '{' and ends with '}'.
 * It does not check if the String is a valid object.
 *
 * @param str a String to check.
 * @return true if the String is a serialized object. false otherwise.
 */
bool AnyParser::isObject(const String &str) {
    if (str.length() < 2) {
        return false;
    }

    return str[0] == OBJECT_OPEN_BRACKET && str[str.length() - 1] == OBJECT_CLOSE_BRACKET;
}

/**
 * @brief Check if the String is a serialized string.
 * This method only checks if the String starts with '"' and ends with '"'.
 * It does not check if the String is a valid ASCII string.
 *
 * @param str a String to check.
 * @return true if the String is a serialized string. false otherwise.
 */
bool AnyParser::isString(const String &str) {
    if (str.length() < 2) {
        return false;
    }

    return str[0] == STRING_BRACKET && str[str.length() - 1] == STRING_BRACKET;
}

/**
 * @brief Check if the string is a float
 *
 * @param str The string to check.
 * @return true if the string is a floating point number.
 */
bool AnyParser::isFloat(const String &str) {
    return abs(fmod(str.toDouble(), 1)) != 0.0;
}

bool AnyParser::isNumber(const String &str) {
    if (str.isEmpty()) {
        return false;
    }

    if (!isdigit(str[0]) && str[0] != '-' && str[0] != '+' && str[0] != '.') {
        return false;
    }

    uint8_t period   = 0;
    uint8_t exponent = 0;

    for (int i = 0; i < str.length(); i++) {
        if (str[i] == '.') {
            period++;
            if (exponent > 0) {
                return false;
            }
            if (period > 1) {
                return false;
            }
            continue;
        }

        if (str[i] == 'e' || str[i] == 'E') {
            exponent++;
            if (exponent > 1) {
                return false;
            }
            continue;
        }

        if (str[i] == '+' && i != 0) {
            return false;
        }

        if (str[i] == '-') {
            if (i != 0 && str[i - 1] != 'e' && str[i - 1] != 'E') {
                return false;
            }
            continue;
        }

        if (!isdigit(str[i])) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Check if the string is a literal.
 *
 * @param str The string to check.
 * @return true if the string is a literal.
 */
bool AnyParser::isLiteral(const String &str) {
    return str == "true" || str == "false" || str == "null";
}

/**
 * @brief Remove insignificant zeros from a string.
 *
 * @param str The string to remove the zeros from.
 * @return The string without the insignificant zeros.
 */
String AnyParser::removeInsignificantZeros(const String &str) {
    String s = str;
    if (s.indexOf(".") != -1) {
        while (s.endsWith("0")) {
            s.remove(s.length() - 1);
        }
        if (s.endsWith(".")) {
            s.remove(s.length() - 1);
        }
    }
    return s;
}

/**
 * @brief Parse a string an return a vector of Any.
 * This method is used by an inheriting Object class to construct its members.
 *
 * @param str is the string to parse.
 * @return A vector of Any.
 */
std::vector<Any> AnyParser::parse(const String &str) {
    std::vector<Any> v;

    int16_t index = 1;
    while (index < str.length() - 1) {
        if (str[index] == AnyParser::SEPARATOR) {
            index++;
            continue;
        }

        if (str[index] == AnyParser::OBJECT_OPEN_BRACKET) {
            int16_t closeIndex = AnyParser::findClosingObjectBracket(str, index);

            if (closeIndex == -1) {
                v.clear();
                return v;
            }

            v.push_back(Any::parse(str.substring(index, closeIndex + 1)));
            index = closeIndex + 1;
            continue;
        }

        if (str[index] == AnyParser::ARRAY_OPEN_BRACKET) {
            int16_t closeIndex = AnyParser::findClosingArrayBracket(str, index);

            if (closeIndex == -1) {
                v.clear();
                return v;
            }

            v.push_back(Any::parse(str.substring(index, closeIndex + 1)));
            index = closeIndex + 1;
            continue;
        }

        if (str[index] == AnyParser::STRING_BRACKET) {
            int32_t closeIndex = AnyParser::findClosingQuote(str, index + 1);

            if (closeIndex == -1) {
                v.clear();
                return v;
            }

            String element = str.substring(index + 1, closeIndex);

            element.replace(String(AnyParser::ESCAPE_STRING_BRACKET), String(AnyParser::STRING_BRACKET));

            v.push_back(element);
            index = closeIndex + 1;
            continue;
        }

        int32_t closeIndex = str.indexOf(AnyParser::SEPARATOR, index);
        if (closeIndex == -1) {
            closeIndex = str.length() - 1;
        }

        String element = str.substring(index, closeIndex);
        index          = closeIndex + 1;

        if (!isLiteral(element) && !isNumber(element)) {
            v.clear();
            return v;
        }

        Any value = parseLiteral(element);
        v.push_back(value);
    }

    return v;
}

/**
 * @brief Parse a string literal and return an Any.
 * If the string is 'true' or 'false' it will return a Boolean.
 * If the string is a number it will return an integer or a Double.
 * If the string is not a literal it will return Null.
 *
 * @param str is the string to parse.
 * @return Any.
 */
Any AnyParser::parseLiteral(const String &str) {
    if (str.isEmpty()) {
        return Any();
    }

    if (str == AnyParser::TRUE) {
        return true;
    }

    if (str == AnyParser::FALSE) {
        return false;
    }

    if (!isdigit(str[0]) && str[0] != '-' && str[0] != '+' && str[0] != '.') {
        return Any();
    }

    if (isFloat(str)) {
        return str.toDouble();
    }

    return parseInt(str);
}

/**
 * @brief Serialize a string.
 * This method will escape the double quotes and add double quotes to the string.
 *
 * @param value The string to serialize.
 * @return The serialized string.
 */
String AnyParser::serialize(const String &value) {
    String result = value;
    result.replace(String(AnyParser::ESCAPE_STRING_BRACKET), String(AnyParser::STRING_BRACKET));
    result.replace(String(AnyParser::STRING_BRACKET), String(AnyParser::ESCAPE_STRING_BRACKET));
    return String(STRING_BRACKET) + result + String(STRING_BRACKET);
}
