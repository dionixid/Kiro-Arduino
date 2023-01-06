#include "SHA1.h"

/**
 * @brief Create a SHA1 instance.
 *
 * @param text is the text to hash.
 */
SHA1::SHA1(const char *text)
    : m_Index(0),
      m_CountBits(0) {
    m_State[0] = 0x67452301;
    m_State[1] = 0xEFCDAB89;
    m_State[2] = 0x98BADCFE;
    m_State[3] = 0x10325476;
    m_State[4] = 0xC3D2E1F0;

    if (text) {
        add(text);
    }
}

/**
 * @brief Create a SHA1 instance.
 *
 * @param str is the text to hash.
 */
SHA1::SHA1(String str)
    : m_Index(0),
      m_CountBits(0) {
    m_State[0] = 0x67452301;
    m_State[1] = 0xEFCDAB89;
    m_State[2] = 0x98BADCFE;
    m_State[3] = 0x10325476;
    m_State[4] = 0xC3D2E1F0;

    add(str.c_str());
}

/**
 * @brief Add a byte to the hash.
 *
 * @param x is the byte to add.
 * @return This instance.
 */
SHA1 &SHA1::add(uint8_t x) {
    _addByteDontCountBits(x);
    m_CountBits += 8;
    return *this;
}

/**
 * @brief Add a character to the hash.
 *
 * @param c is the character to add.
 * @return This instance.
 */
SHA1 &SHA1::add(char c) {
    return add(*(uint8_t *)&c);
}

/**
 * @brief Add a block of data to the hash.
 *
 * @param data is the data to add.
 * @param n is the number of bytes to add.
 * @return This instance.
 */
SHA1 &SHA1::add(const void *data, uint32_t n) {
    if (!data) {
        return *this;
    }

    const uint8_t *ptr = (const uint8_t *)data;

    for (; n && m_Index % sizeof(m_Buffer); n--) {
        add(*ptr++);
    }

    for (; n >= sizeof(m_Buffer); n -= sizeof(m_Buffer)) {
        _processBlock(ptr);
        ptr += sizeof(m_Buffer);
        m_CountBits += sizeof(m_Buffer) * 8;
    }

    for (; n; n--) {
        add(*ptr++);
    }

    return *this;
}

/**
 * @brief Add a string to the hash.
 *
 * @param text is the string to add.
 * @return This instance.
 */
SHA1 &SHA1::add(const char *text) {
    if (!text) {
        return *this;
    }

    return add(text, strlen(text));
}

/**
 * @brief Finalize the hash.
 * This method must be called before calling getHex() or getBase64().
 * After calling finalize() no more data can be added to the hash.
 *
 * @return This instance.
 */
SHA1 &SHA1::finalize() {
    _addByteDontCountBits(0x80);

    while (m_Index % 64 != 56) {
        _addByteDontCountBits(0x00);
    }

    for (int j = 7; j >= 0; j--) {
        _addByteDontCountBits(m_CountBits >> j * 8);
    }
    
    return *this;
}

/**
 * @brief Get the hash as a hex string.
 * 
 * @param hex is the buffer to store the hex string in.
 * @param zero_terminate is true if the string should be zero terminated.
 * @param alphabet is the alphabet to use.
 * @return This instance.
 */
const SHA1 &SHA1::getHex(char *hex, bool zero_terminate, const char *alphabet) const {
    int k = 0;

    for (int i = 0; i < 5; i++) {
        for (int j = 7; j >= 0; j--) {
            hex[k++] = alphabet[(m_State[i] >> j * 4) & 0xf];
        }
    }

    if (zero_terminate) {
        hex[k] = '\0';
    }

    return *this;
}

/**
 * @brief Get the hash as a base64 string.
 * 
 * @param base64 is the buffer to store the base64 string in.
 * @param zero_terminate is true if the string should be zero terminated.
 * @return This instance.
 */
const SHA1 &SHA1::getBase64(char *base64, bool zero_terminate) const {
    static const uint8_t *table = (const uint8_t *)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    uint32_t triples[7]         = {
        ((m_State[0] & 0xffffff00) >> 1 * 8),
        ((m_State[0] & 0x000000ff) << 2 * 8) | ((m_State[1] & 0xffff0000) >> 2 * 8),
        ((m_State[1] & 0x0000ffff) << 1 * 8) | ((m_State[2] & 0xff000000) >> 3 * 8),
        ((m_State[2] & 0x00ffffff) << 0 * 8),
        ((m_State[3] & 0xffffff00) >> 1 * 8),
        ((m_State[3] & 0x000000ff) << 2 * 8) | ((m_State[4] & 0xffff0000) >> 2 * 8),
        ((m_State[4] & 0x0000ffff) << 1 * 8),
    };

    for (int i = 0; i < 7; i++) {
        uint32_t x        = triples[i];
        base64[i * 4 + 0] = table[(x >> 3 * 6) % 64];
        base64[i * 4 + 1] = table[(x >> 2 * 6) % 64];
        base64[i * 4 + 2] = table[(x >> 1 * 6) % 64];
        base64[i * 4 + 3] = table[(x >> 0 * 6) % 64];
    }

    base64[SHA1_BASE64_SIZE - 2] = '=';
    if (zero_terminate) base64[SHA1_BASE64_SIZE - 1] = '\0';
    return *this;
}

/**
 * @brief Get the hash as a hex string.
 * 
 * @return The hex string.
 */
String SHA1::getHexString() {
    char hex[SHA1_HEX_SIZE];
    getHex(hex);
    return String(hex);
}

/**
 * @brief Get the hash as a base64 string.
 * 
 * @return The base64 string.
 */
String SHA1::getBase64String() {
    char base64[SHA1_BASE64_SIZE];
    getBase64(base64);
    return String(base64);
}

/**
 * @brief Add a byte to the hash.
 * This method will not count the byte towards the total number of bits.
 * 
 * @param x 
 */
void SHA1::_addByteDontCountBits(uint8_t x) {
    m_Buffer[m_Index++] = x;
    if (m_Index >= sizeof(m_Buffer)) {
        m_Index = 0;
        _processBlock(m_Buffer);
    }
}

/**
 * @brief Rotate a 32 bit value to the left.
 * 
 * @param x is the value to rotate.
 * @param n is the number of bits to rotate.
 * @return The rotated value.
 */
uint32_t SHA1::_rol32(uint32_t x, uint32_t n) {
    return (x << n) | (x >> (32 - n));
}

/**
 * @brief Make a 32 bit word from 4 bytes.
 * 
 * @param p is the pointer to the bytes.
 * @return The 32 bit word.
 */
uint32_t SHA1::_makeWord(const uint8_t *p) {
    return ((uint32_t)p[0] << 3 * 8) | ((uint32_t)p[1] << 2 * 8) | ((uint32_t)p[2] << 1 * 8)
           | ((uint32_t)p[3] << 0 * 8);
}

/**
 * @brief Process a block of data.
 * 
 * @param r is the round number.
 * @param w is the word array.
 * @param v is the first state variable.
 * @param u is the second state variable.
 * @param x is the third state variable.
 * @param y is the fourth state variable.
 * @param z is the fifth state variable.
 * @param i is the word index.
 */
void SHA1::_shaRound(
    uint8_t r, uint32_t *w, uint32_t &v, uint32_t &u, uint32_t &x, uint32_t &y, uint32_t &z, uint32_t i
) {
    static const uint32_t c0 = 0x5a827999;
    static const uint32_t c1 = 0x6ed9eba1;
    static const uint32_t c2 = 0x8f1bbcdc;
    static const uint32_t c3 = 0xca62c1d6;
    switch (r) {
        case 0:
            z += ((u & (x ^ y)) ^ y) + w[i & 15] + c0 + _rol32(v, 5);
            break;
        case 1:
            w[i & 15] = _rol32(w[(i + 13) & 15] ^ w[(i + 8) & 15] ^ w[(i + 2) & 15] ^ w[i & 15], 1);
            z += ((u & (x ^ y)) ^ y) + w[i & 15] + c0 + _rol32(v, 5);
            break;
        case 2:
            w[i & 15] = _rol32(w[(i + 13) & 15] ^ w[(i + 8) & 15] ^ w[(i + 2) & 15] ^ w[i & 15], 1);
            z += (u ^ x ^ y) + w[i & 15] + c1 + _rol32(v, 5);
            break;
        case 3:
            w[i & 15] = _rol32(w[(i + 13) & 15] ^ w[(i + 8) & 15] ^ w[(i + 2) & 15] ^ w[i & 15], 1);
            z += (((u | x) & y) | (u & x)) + w[i & 15] + c2 + _rol32(v, 5);
            break;
        case 4:
            w[i & 15] = _rol32(w[(i + 13) & 15] ^ w[(i + 8) & 15] ^ w[(i + 2) & 15] ^ w[i & 15], 1);
            z += (u ^ x ^ y) + w[i & 15] + c3 + _rol32(v, 5);
            break;
    }
    u = _rol32(u, 30);
}

/**
 * @brief Process a block of data.
 * 
 * @param ptr is the pointer to the block of data.
 */
void SHA1::_processBlock(const uint8_t *ptr) {
    uint32_t a = m_State[0];
    uint32_t b = m_State[1];
    uint32_t c = m_State[2];
    uint32_t d = m_State[3];
    uint32_t e = m_State[4];

    uint32_t w[16];
    for (int _i = 0; _i < 16; _i++) w[_i] = _makeWord(ptr + _i * 4);

    _shaRound(0, w, a, b, c, d, e, 0);
    _shaRound(0, w, e, a, b, c, d, 1);
    _shaRound(0, w, d, e, a, b, c, 2);
    _shaRound(0, w, c, d, e, a, b, 3);
    _shaRound(0, w, b, c, d, e, a, 4);
    _shaRound(0, w, a, b, c, d, e, 5);
    _shaRound(0, w, e, a, b, c, d, 6);
    _shaRound(0, w, d, e, a, b, c, 7);
    _shaRound(0, w, c, d, e, a, b, 8);
    _shaRound(0, w, b, c, d, e, a, 9);
    _shaRound(0, w, a, b, c, d, e, 10);
    _shaRound(0, w, e, a, b, c, d, 11);
    _shaRound(0, w, d, e, a, b, c, 12);
    _shaRound(0, w, c, d, e, a, b, 13);
    _shaRound(0, w, b, c, d, e, a, 14);
    _shaRound(0, w, a, b, c, d, e, 15);
    _shaRound(1, w, e, a, b, c, d, 16);
    _shaRound(1, w, d, e, a, b, c, 17);
    _shaRound(1, w, c, d, e, a, b, 18);
    _shaRound(1, w, b, c, d, e, a, 19);
    _shaRound(2, w, a, b, c, d, e, 20);
    _shaRound(2, w, e, a, b, c, d, 21);
    _shaRound(2, w, d, e, a, b, c, 22);
    _shaRound(2, w, c, d, e, a, b, 23);
    _shaRound(2, w, b, c, d, e, a, 24);
    _shaRound(2, w, a, b, c, d, e, 25);
    _shaRound(2, w, e, a, b, c, d, 26);
    _shaRound(2, w, d, e, a, b, c, 27);
    _shaRound(2, w, c, d, e, a, b, 28);
    _shaRound(2, w, b, c, d, e, a, 29);
    _shaRound(2, w, a, b, c, d, e, 30);
    _shaRound(2, w, e, a, b, c, d, 31);
    _shaRound(2, w, d, e, a, b, c, 32);
    _shaRound(2, w, c, d, e, a, b, 33);
    _shaRound(2, w, b, c, d, e, a, 34);
    _shaRound(2, w, a, b, c, d, e, 35);
    _shaRound(2, w, e, a, b, c, d, 36);
    _shaRound(2, w, d, e, a, b, c, 37);
    _shaRound(2, w, c, d, e, a, b, 38);
    _shaRound(2, w, b, c, d, e, a, 39);
    _shaRound(3, w, a, b, c, d, e, 40);
    _shaRound(3, w, e, a, b, c, d, 41);
    _shaRound(3, w, d, e, a, b, c, 42);
    _shaRound(3, w, c, d, e, a, b, 43);
    _shaRound(3, w, b, c, d, e, a, 44);
    _shaRound(3, w, a, b, c, d, e, 45);
    _shaRound(3, w, e, a, b, c, d, 46);
    _shaRound(3, w, d, e, a, b, c, 47);
    _shaRound(3, w, c, d, e, a, b, 48);
    _shaRound(3, w, b, c, d, e, a, 49);
    _shaRound(3, w, a, b, c, d, e, 50);
    _shaRound(3, w, e, a, b, c, d, 51);
    _shaRound(3, w, d, e, a, b, c, 52);
    _shaRound(3, w, c, d, e, a, b, 53);
    _shaRound(3, w, b, c, d, e, a, 54);
    _shaRound(3, w, a, b, c, d, e, 55);
    _shaRound(3, w, e, a, b, c, d, 56);
    _shaRound(3, w, d, e, a, b, c, 57);
    _shaRound(3, w, c, d, e, a, b, 58);
    _shaRound(3, w, b, c, d, e, a, 59);
    _shaRound(4, w, a, b, c, d, e, 60);
    _shaRound(4, w, e, a, b, c, d, 61);
    _shaRound(4, w, d, e, a, b, c, 62);
    _shaRound(4, w, c, d, e, a, b, 63);
    _shaRound(4, w, b, c, d, e, a, 64);
    _shaRound(4, w, a, b, c, d, e, 65);
    _shaRound(4, w, e, a, b, c, d, 66);
    _shaRound(4, w, d, e, a, b, c, 67);
    _shaRound(4, w, c, d, e, a, b, 68);
    _shaRound(4, w, b, c, d, e, a, 69);
    _shaRound(4, w, a, b, c, d, e, 70);
    _shaRound(4, w, e, a, b, c, d, 71);
    _shaRound(4, w, d, e, a, b, c, 72);
    _shaRound(4, w, c, d, e, a, b, 73);
    _shaRound(4, w, b, c, d, e, a, 74);
    _shaRound(4, w, a, b, c, d, e, 75);
    _shaRound(4, w, e, a, b, c, d, 76);
    _shaRound(4, w, d, e, a, b, c, 77);
    _shaRound(4, w, c, d, e, a, b, 78);
    _shaRound(4, w, b, c, d, e, a, 79);

    m_State[0] += a;
    m_State[1] += b;
    m_State[2] += c;
    m_State[3] += d;
    m_State[4] += e;
}
