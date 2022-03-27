#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>

/**
 * Allows the registering of handlers for specific commands which will be called if the corresponding command is parsed.
 *
 * A command is always in the format /<command> <arg>
 */
class CommandParser {
public:
    using CommandHandler = std::function<void(const std::string&)>;
    using Logger = std::function<void(const std::string&)>;

    /**
     * Parses the given text for commands and arguments & call eventually registered command handlers
     *
     * @param[in] text The text which shoule be parsed
     */
    void parseCommand(const std::string& text);

    /**
     * Adds a command handler for a specific command. The handler will be called later when the command is used.
     *
     * @param[in] command Command for which the handler shoule be registered
     * @param[in] handler The handler for that command
     */
    void addCommandHandler(const std::string& command, const CommandHandler& handler) { _handlers.insert( {command, handler} ); }

    /**
     * Sets the logger to which logging message are dispatched
     *
     * @param[in] logger
     */
    void setLogger(const Logger& logger) { _logger = logger; }

private:
    void callHandler(const std::string& command, const std::string& arg);

    /**
     * Map with relation of command the corresponding handler
     */
    std::map<const std::string, CommandHandler> _handlers;

    Logger _logger;
};