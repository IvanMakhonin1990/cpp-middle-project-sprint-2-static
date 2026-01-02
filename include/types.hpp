#pragma once

#include <concepts>
#include <cstdint>
#include <string_view>
#include <tuple>

namespace stdx::details {

// ============================================================================
// fixed_string: Шаблонный класс для работы со строками на этапе компиляции
// ============================================================================

template <std::size_t N>
struct fixed_string {
    char data[N] = {};
    static constexpr std::size_t size_value = N;

    constexpr fixed_string() = default;

    template <std::size_t M>
        requires(M <= N)
    constexpr fixed_string(const char (&arr)[M]) {
        for (std::size_t i = 0; i < M; ++i) {
            data[i] = arr[i];
        }
    }

    constexpr fixed_string(const char *begin, const char *end) {
        std::size_t idx = 0;
        while (begin != end && idx < N) {
            data[idx++] = *begin++;
        }
    }

    constexpr std::size_t size() const {
        std::size_t len = 0;
        while (len < N && data[len] != '\0') {
            ++len;
        }
        return len;
    }

    constexpr std::string_view view() const { return std::string_view(data, size()); }

    constexpr bool operator==(const fixed_string &other) const {
        for (std::size_t i = 0; i < N; ++i) {
            if (data[i] != other.data[i]) {
                return false;
            }
        }
        return true;
    }

    constexpr char operator[](std::size_t i) const { return data[i]; }
};

// ============================================================================
// parse_error: Класс для хранения ошибок парсинга
// ============================================================================

struct parse_error : fixed_string<256> {
    constexpr parse_error() = default;

    template <std::size_t M>
    constexpr parse_error(const char (&arr)[M]) : fixed_string<256>(arr) {}

    constexpr parse_error(const char *begin, const char *end) : fixed_string<256>(begin, end) {}
};

// ============================================================================
// scan_result: Класс для хранения результатов парсинга
// ============================================================================

template <typename... Ts>
struct scan_result {
    std::tuple<Ts...> values;

    constexpr scan_result(Ts... args) : values(args...) {}

    constexpr const auto &get_values() const { return values; }

    template <std::size_t I>
    constexpr auto get() const {
        return std::get<I>(values);
    }
};

// ============================================================================
// Концепты для проверки допустимых типов
// ============================================================================

template <typename T>
concept ParseableType = requires {
    typename std::enable_if_t<std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t> ||
                              std::is_same_v<T, int64_t> || std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
                              std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t> ||
                              std::is_same_v<T, std::string_view>>;
};

}  // namespace stdx::details
