#ifndef _UI_EVENTS_HPP_
#define _UI_EVENTS_HPP_

#include <cstdint>
#include <mpi.h>
#include <ostream>

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

class UiEvent {
private:
    enum class EventType : uint8_t {
        CloseWindow = 1,
        AnimationStart = 2,
        AnimationStop = 3,
        TimestepIncrement = 4,
        TimestepDecrement = 5,
        Advance = 6,

        Noop = 0,
    };

    EventType data[1] = { Noop };

public:
    using enum EventType;

    constexpr static int TAG = 'E';

    inline constexpr UiEvent() { data[0] = Noop; }
    inline constexpr UiEvent(EventType event) { data[0] = event; }

    inline int send(int dest, MPI_Comm comm) const {
        return MPI_Send(data, 1, MPI_UINT8_T, dest, UiEvent::TAG, comm);
    }

    inline int recv(int source, MPI_Comm comm, MPI_Status * status) {
        return MPI_Recv(data, 1, MPI_UINT8_T, source, UiEvent::TAG, comm, status);
    }

    inline constexpr void operator=(EventType event) { data[0] = event; }

    inline constexpr bool operator==(EventType event) const { return data[0] == event; }

    template <typename char_t>
    friend inline std::basic_ostream<char_t, std::char_traits<char_t>> &
        operator<<(std::basic_ostream<char_t, std::char_traits<char_t>> & os, UiEvent event) {
        switch (static_cast<int>(event.data[0])) {
            case 1: os << "CloseWindow"; break;
            case 2: os << "AnimationStart"; break;
            case 3: os << "AnimationStop"; break;
            case 4: os << "TimestepIncrement"; break;
            case 5: os << "TimestepDecrement"; break;
            case 6: os << "Advance"; break;
            case 0: os << "Noop"; break;
        }
        return os;
    }
};

constexpr const int TEST = sizeof(UiEvent::CloseWindow);

#endif
