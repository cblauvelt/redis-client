#include "redis/errors.hpp"

namespace redis {

namespace detail {

struct redis_error_code_category : std::error_category {
    const char* name() const noexcept override { return "error_code"; }

    std::string message(int ev) const override {
        switch (static_cast<error_code>(ev)) {
        case error_code::no_error:
            return "Success";
        case error_code::not_supported:
            return "The requested action is not yet supported";
        case error_code::wrong_type:
            return "An conversion to an invalid type was requested";
        default:
            return "(unrecognized error_code)";
        }
    }
};

struct redis_client_error_code_category : std::error_category {
    const char* name() const noexcept override { return "client_error_code"; }

    std::string message(int ev) const override {
        switch (static_cast<client_error_code>(ev)) {
        case client_error_code::no_error:
            return "Success";
        case client_error_code::error:
            return "The server returned an error";
        case client_error_code::disconnected:
            return "The client was disconnected";
        case client_error_code::write_error:
            return "There was an error while writing the command to the server";
        case client_error_code::read_error:
            return "There was an error while reading a response from the "
                   "server";
        case client_error_code::response_command_mismatch:
            return "There was a mismatch between the number of commands sent "
                   "and the number of responses received";
        case client_error_code::client_stopped:
            return "The client has been stopped and no further requests will "
                   "succeed";
        default:
            return "(unrecognized client_error_code)";
        }
    }
};

struct redis_subscriber_error_code_category : std::error_category {
    const char* name() const noexcept override {
        return "redis_subscriber_error_codeCategory";
    }

    std::string message(int ev) const override {
        switch (static_cast<subscriber_error_code>(ev)) {
        case subscriber_error_code::no_error:
            return "Success";
        case subscriber_error_code::error:
            return "The server returned an error";
        case subscriber_error_code::disconnected:
            return "The client was disconnected";
        case subscriber_error_code::write_error:
            return "There was an error while writing the command to the server";
        case subscriber_error_code::read_error:
            return "There was an error while reading a response from the "
                   "server";
        case subscriber_error_code::response_command_mismatch:
            return "There was a mismatch between the number of commands sent "
                   "and the number of responses received";
        case subscriber_error_code::bad_reply:
            return "The reply did not match what was expected.";
        default:
            return "(unrecognized subscriber_error_code)";
        }
    }
};

struct parse_error_codeCategory : std::error_category {
    const char* name() const noexcept override {
        return "parse_error_codeCategory";
    }

    std::string message(int ev) const override {
        switch (static_cast<parse_error_code>(ev)) {
        case parse_error_code::no_error:
            return "Success";
        case parse_error_code::eof:
            return "The end of the buffer was reached unexpectedly";
        case parse_error_code::out_of_range:
            return "The parsed number was too large for the container";
        case parse_error_code::malformed_message:
            return "The message did not meet the Redis standard";
        default:
            return "(unrecognized parse_error_code)";
        }
    }
};

const redis_error_code_category the_redis_error_code_category{};
const redis_client_error_code_category the_redis_client_error_code_category{};
const redis_subscriber_error_code_category
    the_redis_subscriber_error_code_category{};
const redis_client_error_code_category theparse_error_code_category{};

} // namespace detail

std::error_code make_error_code(error_code e) {
    return {static_cast<int>(e), detail::the_redis_error_code_category};
}

std::error_code make_error_code(client_error_code e) {
    return {static_cast<int>(e), detail::the_redis_client_error_code_category};
}

std::error_code make_error_code(subscriber_error_code e) {
    return {static_cast<int>(e),
            detail::the_redis_subscriber_error_code_category};
}

std::error_code make_error_code(parse_error_code e) {
    return {static_cast<int>(e), detail::theparse_error_code_category};
}

} // namespace redis