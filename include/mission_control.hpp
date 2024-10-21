#pragma once

#define CONSOLE_LOG_ONLY_MODE

#include <libhal/serial.hpp>
#include <array>
#include <math.hpp>
#include <vec3.hpp>
#include <cstdio>

struct mission_control_data_frame {
    float time;
    math::vec3 position;
    math::vec3 velocity;
    math::vec3 acceleration;
    math::vec3 body_acceleration;

    math::quarternion orientation;
    math::vec3 body_angular_rates;

    float dt;
};

struct mission_control {
    constexpr static std::size_t TX_BUFFER_SIZE = 512;
    std::array<hal::byte, TX_BUFFER_SIZE> m_tx_buffer;
    std::size_t m_tx_i = 0;
    hal::serial& m_console;
    mission_control(hal::serial& p_console) : m_console(p_console) {

    }

    enum frame_bytes : std::uint8_t {
        START_OF_FRAME=0x01, // Start of header
        END_OF_FRAME=0x04, // End of transmission
        ESCAPE=0x27,
    };

    inline void flush_transmit_buffer() {
        // for(const auto& b : std::span(m_tx_buffer).subspan(0, m_tx_i)) {
        //     (*(std::uint32_t*)0x4000'C000) = b;
        // }
        // m_console.write(std::span(m_tx_buffer).subspan(0, m_tx_i));
    }

    inline void queue_transmit_byte(hal::byte b) {
        // if(m_tx_i == TX_BUFFER_SIZE) flush_transmit_buffer();
        // m_tx_buffer[m_tx_i] = b;
        // m_tx_i++;
        // hal::print<128>(m_console, "%c", b);
        m_console.write(std::array<hal::byte, 1> { b });

    }

    inline void start_frame() {
        queue_transmit_byte(START_OF_FRAME);
    }

    inline void send_byte(std::uint8_t b) {
        switch(b) {
            case START_OF_FRAME:
            case END_OF_FRAME:
            case ESCAPE:
                queue_transmit_byte(ESCAPE);
                break;
            default:
                break;
        }
        queue_transmit_byte(b);
    }

    inline void end_frame() {
        queue_transmit_byte(END_OF_FRAME);
    }

    template <typename T>
    void send(T x) {
        union {
            T a;
            std::uint8_t bytes[sizeof(T)];
        } thing;
        thing.a = x;
        for(std::size_t i = 0; i < sizeof(T); i ++) {
            send_byte(thing.bytes[i]);
        }
    }

    inline void send_v(math::vec3 x) {
        send(x.x);
        send(x.y);
        send(x.z);
    }
    inline void send_q(math::quarternion x) {
        send(x.x);
        send(x.y);
        send(x.z);
        send(x.w);
    }


    inline void send_data_frame(const mission_control_data_frame& p_frame) {
        #ifdef CONSOLE_LOG_ONLY_MODE
        return;
        #endif
        start_frame();
        send('D'); // 0
        send(p_frame.time);                 // 1 (4)
        send_v(p_frame.position);           // 5 (12)
        send_v(p_frame.velocity);           // 17 (12)
        send_v(p_frame.acceleration);       // 29 (12)
        send_v(p_frame.body_acceleration);  // 41 (12)
        send_q(p_frame.orientation);        // 53 (16)
        send_v(p_frame.body_angular_rates); // 69 (12)
        send(p_frame.dt); // 81 (4)
        end_frame();
        flush_transmit_buffer();
    }

    inline void log(std::span<const char> p_data) {
        log_with_priority('L', p_data);
    }
    template<std::size_t N, class ...T>
    void log(const char * fmt, const T&... x) {
        char buf[N];
        int n = std::snprintf(buf, N, fmt, x...);
        for(int i = 0; i < n; i ++) {
            send(buf[i]);
        }
        log_with_priority('L', std::span<const char>(buf, n));
    }

    inline void info(std::span<const char> p_data) {
        log_with_priority('L', p_data);
    }

    template<std::size_t N, class ...T>
    void info(const char * fmt, const T&... x) {
        char buf[N];
        int n = std::snprintf(buf, N, fmt, x...);
        log_with_priority('L', std::span<const char>(buf, n));
    }

    inline void debug(std::span<const char> p_data) {
        log_with_priority('L', p_data);
    }

    template<std::size_t N, class ...T>
    void debug(const char * fmt, const T&... x) {
        char buf[N];
        int n = std::snprintf(buf, N, fmt, x...);
        log_with_priority('L', std::span<const char>(buf, n));
    }

    inline void warn(std::span<const char> p_data) {
        log_with_priority('L', p_data);
    }

    template<std::size_t N, class ...T>
    void warn(const char * fmt, const T&... x) {
        char buf[N];
        int n = std::snprintf(buf, N, fmt, x...);
        log_with_priority('L', std::span<const char>(buf, n));
    }

    inline void critical(std::span<const char> p_data) {
        log_with_priority('L', p_data);
    }

    template<std::size_t N, class ...T>
    void critical(const char * fmt, const T&... x) {
        char buf[N];
        int n = std::snprintf(buf, N, fmt, x...);
        log_with_priority('L', std::span<const char>(buf, n));
    }

    inline void log_with_priority(char p_prior, std::span<const char> p_msg) {
        start_frame();
        #ifndef CONSOLE_LOG_ONLY_MODE
        send(p_prior);
        #endif
        for(const auto& byte : p_msg) {
            send(byte);
        }
        #ifdef CONSOLE_LOG_ONLY_MODE
        send('\n');
        #endif
        end_frame();
        flush_transmit_buffer();
    }
};

// struct mission_control_log_frame {
//     std::int64_t time;

// };