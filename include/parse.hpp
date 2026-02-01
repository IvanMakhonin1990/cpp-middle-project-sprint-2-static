#pragma once

#include <charconv>
#include <cstdint>
#include <string_view>

#include "types.hpp"

namespace stdx::details {

// ============================================================================
// Парсинг значений конкретных типов
// ============================================================================

template <typename T>
    requires std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t> ||
             std::is_same_v<T, int64_t>
consteval T parse_value(std::string_view str) {
    T result = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    static_assert(ec == std::errc(), "Parse error: invalid integer format");
    return result;
}

template <typename T>
    requires std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> ||
             std::is_same_v<T, uint64_t>
consteval T parse_value(std::string_view str) {
    T result = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    static_assert(ec == std::errc(), "Parse error: invalid integer format");
    return result;
}

template <typename T>
    requires std::is_same_v<T, std::string_view>
consteval T parse_value(std::string_view str) {
    return str;
}

// ============================================================================
// Helper: найти позицию N-го плейсхолдера в формате
// Возвращает пару (start_pos, end_pos) где {%d} или {%s}
// ============================================================================

consteval std::pair<std::size_t, std::size_t> find_placeholder_at(std::string_view fmt, std::size_t placeholder_index) {
    std::size_t count = 0;
    std::size_t i = 0;

    while (i < fmt.size()) {
        // Ищем открывающую скобку
        if (fmt[i] == '{' && i + 3 < fmt.size() && fmt[i + 1] == '%') {
            if (count == placeholder_index) {
                // Нашли нужный плейсхолдер
                return {i, i + 4};  // {%d} или {%s} - 4 символа
            }
            count++;
            i += 4;  // Пропускаем {%X}
        } else {
            i++;
        }
    }
    return {fmt.size(), fmt.size()};  // Не найден
}

// ============================================================================
// Главная функция парсинга одного плейсхолдера
// ============================================================================

template <std::size_t I, fixed_string fmt, fixed_string source, typename T>
consteval T parse_input() {
    static_assert(ParseableType<T>, "Type T is not supported for parsing. "
                                    "Supported types: int8_t, int16_t, int32_t, int64_t, "
                                    "uint8_t, uint16_t, uint32_t, uint64_t, std::string_view");

    auto fmt_sv = std::string_view(fmt.data, fmt.size());
    auto src_sv = std::string_view(source.data, source.size());

    auto [curr_ph_start, curr_ph_end] = find_placeholder_at(fmt_sv, I);
    auto [next_ph_start, next_ph_end] = find_placeholder_at(fmt_sv, I + 1);

    auto before_sep = fmt_sv.substr(0, curr_ph_start);

    auto after_start = curr_ph_end;
    auto after_end = (next_ph_start < fmt_sv.size()) ? next_ph_start : fmt_sv.size();
    auto after_sep = fmt_sv.substr(after_start, after_end - after_start);

    std::size_t value_start = 0;

    if (I == 0) {
        value_start = before_sep.size();
    } else {
        auto [prev_ph_start, prev_ph_end] = find_placeholder_at(fmt_sv, I - 1);
        auto between_sep = fmt_sv.substr(prev_ph_end, curr_ph_start - prev_ph_end);

        std::size_t search_pos = 0;
        for (std::size_t j = 0; j < I; ++j) {
            auto pos = src_sv.find(between_sep, search_pos);
            if (pos != std::string_view::npos) {
                search_pos = pos + between_sep.size();
            }
        }
        value_start = search_pos;
    }

    std::size_t value_end = src_sv.size();
    if (after_sep.size() > 0) {
        auto pos = src_sv.find(after_sep, value_start);
        if (pos != std::string_view::npos) {
            value_end = pos;
        }
    }

    auto to_parse = src_sv.substr(value_start, value_end - value_start);
    return parse_value<T>(to_parse);
}

}  // namespace stdx::details
