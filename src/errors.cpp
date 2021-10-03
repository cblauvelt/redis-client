#include "errors.hpp"

namespace redis {

namespace detail {

struct redis_error_code_category : std::error_category {
    const char* name() const noexcept override { return "redis_error_code"; }

    std::string message(int ev) const override {
        switch (static_cast<redis_error_code>(ev)) {
        case redis_error_code::no_error:
            return "Success";
        case redis_error_code::not_supported:
            return "The requested action is not yet supported";
        case redis_error_code::wrong_type:
            return "An conversion to an invalid type was requested";
        default:
            return "(unrecognized redis_error_code)";
        }
    }
};

struct redis_client_error_codeCategory : std::error_category {
    const char* name() const noexcept override {
        return "redis_client_error_code";
    }

    std::string message(int ev) const override {
        switch (static_cast<redis_client_error_code>(ev)) {
        case redis_client_error_code::no_error:
            return "Success";
        case redis_client_error_code::redis_error:
            return "The server returned an error";
        case redis_client_error_code::disconnected:
            return "The client was disconnected";
        case redis_client_error_code::write_error:
            return "There was an error while writing the command to the server";
        case redis_client_error_code::read_error:
            return "There was an error while reading a response from the "
                   "server";
        case redis_client_error_code::response_command_mismatch:
            return "There was a mismatch between the number of commands sent "
                   "and the number of responses received";
        default:
            return "(unrecognized redis_client_error_code)";
        }
    }
};

struct redis_subscriber_error_codeCategory : std::error_category {
    const char* name() const noexcept override {
        return "redis_subscriber_error_codeCategory";
    }

    std::string message(int ev) const override {
        switch (static_cast<redis_subscriber_error_code>(ev)) {
        case redis_subscriber_error_code::no_error:
            return "Success";
        case redis_subscriber_error_code::redis_error:
            return "The server returned an error";
        case redis_subscriber_error_code::disconnected:
            return "The client was disconnected";
        case redis_subscriber_error_code::write_error:
            return "There was an error while writing the command to the server";
        case redis_subscriber_error_code::read_error:
            return "There was an error while reading a response from the "
                   "server";
        case redis_subscriber_error_code::response_command_mismatch:
            return "There was a mismatch between the number of commands sent "
                   "and the number of responses received";
        case redis_subscriber_error_code::bad_reply:
            return "The reply did not match what was expected.";
        default:
            return "(unrecognized redis_subscriber_error_code)";
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
const redis_client_error_codeCategory the_redis_client_error_codeCategory{};
const redis_subscriber_error_codeCategory
    the_redis_subscriber_error_codeCategory{};
const redis_client_error_codeCategory theparse_error_codeCategory{};

} // namespace detail

std::error_code make_error_code(redis_error_code e) {
    return {static_cast<int>(e), detail::the_redis_error_code_category};
}

std::error_code make_error_code(redis_client_error_code e) {
    return {static_cast<int>(e), detail::the_redis_client_error_codeCategory};
}

std::error_code make_error_code(redis_subscriber_error_code e) {
    return {static_cast<int>(e),
            detail::the_redis_subscriber_error_codeCategory};
}

std::error_code make_error_code(parse_error_code e) {
    return {static_cast<int>(e), detail::theparse_error_codeCategory};
}

} // namespace redis