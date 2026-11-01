/* ========================================================================== */
/*  IRCmacros.hpp - IRC numeric reply codes and error codes (RFC 2812)        */
/* ========================================================================== */

#ifndef IRCMACROS_HPP
# define IRCMACROS_HPP

# define CRLF "\r\n"
# define SPACE " "

/* -------------------------------------------------------------------------- */
/*  Connection Registration Replies                                           */
/* -------------------------------------------------------------------------- */

# define RPL_NOTICE                 "NOTICE"

# define RPL_WELCOME                "001"   // :Welcome to the IRC Network
# define RPL_YOURHOST               "002"   // :Your host is <server>, running version <ver>
# define RPL_CREATED                "003"   // :This server was created <date>
# define RPL_MYINFO                 "004"   // <server> <version> <user modes> <chan modes>
# define RPL_NEWNICK                "005"   // :Your new nickname is <nick>

/* -------------------------------------------------------------------------- */
/*  Quit Reply                                                                */
/* -------------------------------------------------------------------------- */

# define RPL_QUIT                   "101"   // :Goodbye!

/* -------------------------------------------------------------------------- */
/*  User Mode Reply                                                           */
/* -------------------------------------------------------------------------- */

# define RPL_UMODEIS                "221"   // <client> <mode>

/* -------------------------------------------------------------------------- */
/*  CAP / LIST Replies                                                        */
/* -------------------------------------------------------------------------- */

# define RPL_CAP                    "302"   // CAP reply
# define RPL_WHOISUSER              "311"   // WHOIS user info
# define RPL_WHOISSERVER            "312"   // WHOIS server info
# define RPL_WHOISOPERATOR          "313"   // WHOIS operator status
# define RPL_ENDOFWHO               "315"   // End of WHO list
# define RPL_WHOISIDLE              "317"   // WHOIS idle info
# define RPL_ENDOFWHOIS             "318"   // End of WHOIS list
# define RPL_WHOISCHANNELS          "319"   // WHOIS channels
# define RPL_LISTSTART              "321"   // Channel :Users Name
# define RPL_LIST                   "322"   // <channel> <count> :<topic>
# define RPL_LISTEND                "323"   // :End of /LIST

/* -------------------------------------------------------------------------- */
/*  Channel Mode Replies                                                      */
/* -------------------------------------------------------------------------- */

# define RPL_CHANNELMODEIS          "324"   // <channel> <mode> <params>

/* -------------------------------------------------------------------------- */
/*  Topic Replies                                                             */
/* -------------------------------------------------------------------------- */

# define RPL_NOTOPIC                "331"   // <channel> :No topic is set
# define RPL_TOPIC                  "332"   // <channel> :<topic>

/* -------------------------------------------------------------------------- */
/*  Invite Reply                                                              */
/* -------------------------------------------------------------------------- */

# define RPL_INVITING               "341"   // <nick> <channel>
# define RPL_INVITED                "345"   // <client> <channel>

/* -------------------------------------------------------------------------- */
/*  Names / WHO Replies                                                       */
/* -------------------------------------------------------------------------- */

# define RPL_WHOREPLY               "352"   // WHO reply
# define RPL_NAMREPLY               "353"   // = <channel> :<nick list>
# define RPL_ENDOFNAMES             "366"   // <channel> :End of /NAMES list
# define RPL_ENDOFMODES             "368"   // End of channel mode list

/* -------------------------------------------------------------------------- */
/*  Operator Reply                                                            */
/* -------------------------------------------------------------------------- */

# define RPL_YOUREOPER              "381"   // :You are now an IRC operator

/* -------------------------------------------------------------------------- */
/*  PING / PONG                                                               */
/* -------------------------------------------------------------------------- */

# define RPL_PING                   "PONG"

/* ========================================================================== */
/*  Error Replies                                                             */
/* ========================================================================== */

# define ERR_NOSUCHNICK             "401"   // <nick> :No such nick/channel
# define ERR_NOSUCHSERVER           "402"   // <server> :No such server
# define ERR_NOSUCHCHANNEL          "403"   // <channel> :No such channel
# define ERR_CANNOTSENDTOCHAN       "404"   // <channel> :Cannot send to channel
# define ERR_TOOMANYCHANNELS        "405"   // <channel> :Too many channels
# define ERR_WASNOSUCHNICK          "406"   // <nick> :There was no such nickname
# define ERR_TOOMANYTARGETS         "407"   // <target> :Duplicate recipients
# define ERR_NORECIPIENT            "411"   // :No recipient given (PRIVMSG)
# define ERR_NOTEXTTOSEND           "412"   // :No text to send
# define ERR_UNKNOWNCOMMAND         "421"   // <command> :Unknown command
# define ERR_NONICKNAMEGIVEN        "431"   // :No nickname given
# define ERR_ERRONEUSNICKNAME       "432"   // <nick> :Erroneous nickname
# define ERR_NICKNAMEINUSE          "433"   // <nick> :Nickname is already in use
# define ERR_USERNOTINCHANNEL       "441"   // <nick> <channel> :They aren't on that channel
# define ERR_NOTONCHANNEL           "442"   // <channel> :You're not on that channel
# define ERR_USERONCHANNEL          "443"   // <nick> <channel> :is already on channel
# define ERR_NOTREGISTERED          "451"   // :You have not registered
# define ERR_NEEDMOREPARAMS         "461"   // <command> :Not enough parameters
# define ERR_ALREADYREGISTRED       "462"   // :You may not reregister
# define ERR_PASSWDMISMATCH         "464"   // :Password incorrect
# define ERR_CHANNELISFULL          "471"   // <channel> :Cannot join channel (+l)
# define ERR_UNKNOWNMODE            "472"   // <char> :Unknown MODE flag
# define ERR_INVITEONLYCHAN         "473"   // <channel> :Cannot join channel (+i)
# define ERR_BADCHANNELKEY          "475"   // <channel> :Cannot join channel (+k)
# define ERR_NOPRIVILEGES           "481"   // :Permission Denied
# define ERR_CHANOPRIVSNEEDED       "482"   // <channel> :You're not channel operator
# define ERR_UMODEUNKNOWNFLAG       "501"   // :Unknown MODE flag
# define ERR_USERSDONTMATCH         "502"   // :Cannot change mode for other users

#endif
