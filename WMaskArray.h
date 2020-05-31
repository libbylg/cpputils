#ifndef WMASKARRAY_H
#define WMASKARRAY_H


#include <cinttypes>

#include "assert.h"

#ifndef Q_ASSERT
#define Q_ASSERT assert
#endif


template <int MASKWIDTH = 2>
class WMaskArray
{
private:
    enum : uint8_t {
        MASK = ((uint8_t(1) << MASKWIDTH) - 1),     //  The basic mask
        MOD = ((sizeof(uint8_t) * 8) / MASKWIDTH),  //  How many value can be store in uint8_t
        VAL_UPLIMIT = (uint8_t(1) << MASKWIDTH),    //  The value which need to be less than
    };
    enum : int {
        CAP_DEF = 64 * MOD,      //  The size of the default capcity
        ALIGN_WIDTH = 64 * MOD,  //  Align to 64 bytes
    };
    static_assert(((MASKWIDTH == 2) || (MASKWIDTH == 4)), "重要:请确保 uint8_t 类型的 bit 位的总数是 WIDTH 的整数倍");

    WMaskArray(const WMaskArray& src) = delete;
    WMaskArray& operator=(const WMaskArray& src) = delete;

public:
    WMaskArray(int newCap = 64 * MOD)
    {
        cap = 0;
        len = 0;
        data = nullptr;
        reserve(newCap);
    }

    WMaskArray(WMaskArray&& src)
    {
        uint8_t* old = data;
        cap = src.cap;
        len = src.len;
        data = src.data;
        src.cap = 0;
        src.len = 0;
        src.data = nullptr;
        delete[] old;
    }

    WMaskArray& operator=(WMaskArray&& src)
    {
        if (&src == this) {
            return *this;
        }
        uint8_t* old = data;
        cap = src.cap;
        len = src.len;
        data = src.data;
        src.cap = 0;
        src.len = 0;
        src.data = nullptr;
        delete[] old;
        return *this;
    }

    int resize(int newSize)
    {
        if (newSize > cap) {
            int ret = inc_cap(newSize);
            if (0 != ret) {
                return ret;
            }
        }
        len = newSize;
        return 0;
    }

    int resize(int newSize, uint8_t val)
    {
        Q_ASSERT(val < VAL_UPLIMIT);
        int oldLen = len;
        if (0 != resize(newSize)) {
            return -1;
        }
        fill(val, oldLen, len);
        return 0;
    }

    inline int reserve(int newCap)
    {
        return inc_cap(newCap);
    }

    inline int size() const
    {
        return len;
    }

    inline void push(uint8_t val)
    {
        Q_ASSERT(val < VAL_UPLIMIT);
        append(val);
    }

    inline uint8_t pop()
    {
        Q_ASSERT(len > 0);
        return getimpl(--len);
    }

    inline void append(uint8_t val)
    {
        Q_ASSERT(val < VAL_UPLIMIT);
        if (len == cap) {
            if (0 != inc_cap(0)) {
                return;
            }
        }
        setimpl(len++, val);
    }

    inline uint8_t last() const
    {
        Q_ASSERT(len > 0);
        return getimpl(len - 1);
    }

    inline uint8_t first() const
    {
        Q_ASSERT(len > 0);
        return getimpl(0);
    }

    inline uint8_t at(int index) const
    {
        Q_ASSERT((index >= 0) && (index < len));
        return getimpl(index);
    }

    inline void set(int index, uint8_t val)
    {
        Q_ASSERT(val < VAL_UPLIMIT);
        Q_ASSERT((index >= 0) && (index < len));
        setimpl(index, val);
    }

    inline uint8_t get(int index) const
    {
        Q_ASSERT((index >= 0) && (index < len));
        return getimpl(index);
    }

    inline void fill(uint8_t val, int begin)
    {
        Q_ASSERT(val < VAL_UPLIMIT);
        fill(val, begin, len);
    }

    void fill(uint8_t val, int begin, int end)
    {
        Q_ASSERT(val < VAL_UPLIMIT);
        if (begin < 0) {
            begin = 0;
            end = len;
        } else if (end < 0) {
            end = len;
        }
        Q_ASSERT((begin >= 0) && (begin < len));
        Q_ASSERT((end >= 0) && (end <= len));
        Q_ASSERT(end > begin);

        if (begin == end) {
            return;
        }

        int byteIndexBegin = (begin / MOD);
        int bitsIndexBegin = (begin % MOD) * 2;
        int byteIndexEnd = (end / MOD);
        int bitsIndexEnd = (end % MOD) * 2;

        val = val & MASK;

        //  The deff-device
        switch (bitsIndexBegin) {
            case 1:
                set(byteIndexBegin * MOD + 1, val);
            case 2:
                set(byteIndexBegin * MOD + 2, val);
            case 3:
                set(byteIndexBegin * MOD + 3, val);
            default:
                break;
        }

        if (byteIndexEnd > bitsIndexBegin) {
            uint8_t byteVal = val;
            for (int i = 0; i < MOD; i++) {
                byteVal = (byteVal << MASKWIDTH) | val;
            }
            memset(data + bitsIndexBegin, byteVal, byteIndexEnd - byteIndexBegin);
        }

        //  The deff-device
        switch (bitsIndexEnd) {
            case 3:
                set(byteIndexEnd * MOD + 3, val);
            case 2:
                set(byteIndexEnd * MOD + 2, val);
            case 1:
                set(byteIndexEnd * MOD + 1, val);
            default:
                break;
        }
    }

    void clear()
    {
        len = 0;
    }

private:
    inline void setimpl(int index, uint8_t v)
    {
        int byteIndex = index / MOD;                  // 6 : 1=(6 / (8/2)) xx-xx-xx-xx xx-##-xx-xx xx-xx-xx-xx
        int bitsIndex = ((index % MOD) * MASKWIDTH);  // 6 : 4=((6 % (8/2))* 2)
        uint8_t bitsMask = MASK << bitsIndex;
        data[byteIndex] = (data[byteIndex] & (~bitsMask)) | ((v << bitsIndex) & bitsMask);
    }

    inline uint8_t getimpl(int index) const
    {
        int byteIndex = index / MOD;                  // 6 : 1=(6 / (8/2)) xx-xx-xx-xx xx-##-xx-xx xx-xx-xx-xx
        int bitsIndex = ((index % MOD) * MASKWIDTH);  // 6 : 4=((6 % (8/2))* 2)
        uint8_t bitsMask = MASK << bitsIndex;
        return ((data[byteIndex] & bitsMask) >> bitsIndex);
    }

private:
    static inline int ASBYTE(int size)
    {
        return (size / MOD) + (!!(size % MOD));
    }

    inline int tidy(int size)
    {
        return ((size / ALIGN_WIDTH + !!(size % ALIGN_WIDTH)) * ALIGN_WIDTH);
    }

    inline int inc_cap(int newCap)
    {
        if (newCap <= 0) {
            newCap = cap + ((cap < (4096 * MOD)) ? (64 * MOD) : (4096 * MOD));
        }

        if (newCap <= cap) {
            return 0;
        }

        newCap = tidy(newCap);

        uint8_t* newData = (uint8_t*)malloc(ASBYTE(newCap));
        if (nullptr == newData) {
            return -1;
        }

        if (nullptr != data) {
            memcpy(newData, data, ASBYTE(len));
        }

        uint8_t* oldData = data;
        cap = newCap;
        data = newData;
        free(oldData);
        return 0;
    }

private:
    uint8_t* data;
    int len;
    int cap;
};


#endif  // WMASKARRAY_H
