/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 23/09/2026 by @author Tsukini

File Name:
##  @file Config.hpp

File Description:
##  Tools used to parse a libconfig content and keep it alive
##  during the use of the extracted setting
\**************************************************************/

#ifndef CONFIG_H
    #define CONFIG_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <libconfig.h++>    // libconfig::Config, libconfig::Setting
    #include <string>           // std::string

namespace tests::tools { // namespace start
//----------------------------------------------------------------//
/* CLASS */

/* keep the libconfig::Config alive as long as the setting is used */
class Config {
    private:
        libconfig::Config _cfg;

    public:
        // ------------ Function ---------- //
        // parse the content and return the setting `key` of the root (throw libconfig exceptions on invalid content/key)
        const libconfig::Setting& parse(const std::string& content, const std::string& key) {
            this->_cfg.readString(content);
            return this->_cfg.getRoot()[key.c_str()];
        };

        // ---------- Constructor --------- //
        Config() = default;
        Config(const Config& other) = delete;
        Config(Config&& other) = delete;
};

} // namespace end
#endif /* CONFIG_H */
