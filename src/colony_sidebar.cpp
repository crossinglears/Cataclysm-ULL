#include "colony_sidebar.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <imgui/imgui.h>

#include "avatar.h"
#include "cata_imgui.h"
#include "colony_camera.h"
#include "color.h"
#include "coordinates.h"
#include "display.h"
#include "messages.h"
#include "options.h"
#include "panels.h"
#include "string_formatter.h"
#include "translations.h"
#include "weather.h"
#include "weather_type.h"

namespace
{

/** Match data/json/ui/sidebar-cull-colony.json width. */
constexpr int colony_sidebar_chars = 44;
constexpr int colony_overmap_w = 21;
constexpr int colony_overmap_h = 21;
constexpr size_t colony_log_message_count = 40;

tripoint_abs_omt colony_camera_omt()
{
    const avatar &u = get_avatar();
    return project_to<coords::omt>( u.pos_abs() + u.view_offset );
}

class colony_sidebar_ui : public cataimgui::window
{
    public:
        colony_sidebar_ui() : cataimgui::window( _( "Colony sidebar" ),
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav |
                    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollbar ) {
            // Stay behind modal leadership panels (faction UI, uilist, etc.).
            force_to_back = true;
        }

    protected:
        void draw_controls() override;
        cataimgui::bounds get_bounds() override;
};

void colony_sidebar_ui::draw_controls()
{
    avatar &u = get_avatar();
    const tripoint_abs_omt cam_omt = colony_camera_omt();

    const std::string place = display::current_position_text( cam_omt );
    std::pair<std::string, nc_color> weather;
    if( cam_omt.z() < 0 ) {
        weather = { _( "Underground" ), c_light_gray };
    } else {
        const weather_manager &wm = get_weather();
        weather = { wm.weather_id->name.translated(), wm.weather_id->color };
    }
    const std::string date = display::date_string();
    const std::string time = display::time_string( u );
    const auto wind = display::wind_text_color( u );
    const std::string temp = display::get_temp( u );

    auto labeled = []( const char *label, const std::string & value, nc_color col = c_white ) {
        ImGui::TextUnformatted( label );
        ImGui::SameLine( 0.0f, 0.0f );
        ImGui::TextUnformatted( ": " );
        ImGui::SameLine( 0.0f, 0.0f );
        cataimgui::draw_colored_text( value, col );
    };

    labeled( _( "Place" ), place );
    labeled( _( "Weather" ), weather.first, weather.second );
    labeled( _( "Date" ), date );
    labeled( _( "Time" ), time );
    labeled( _( "Wind" ), wind.first, wind.second );
    labeled( _( "Temp" ), temp );

    ImGui::Separator();

    // Log fills remaining space above the overmap.
    const float map_reserve = str_height_to_pixels( colony_overmap_h + 2 ) +
                              ImGui::GetTextLineHeightWithSpacing();
    const float log_h = std::max( ImGui::GetTextLineHeightWithSpacing() * 4.0f,
                                  ImGui::GetContentRegionAvail().y - map_reserve );

    ImGui::TextUnformatted( _( "Log" ) );
    if( ImGui::BeginChild( "##colony_sidebar_log", ImVec2( 0.0f, log_h ), ImGuiChildFlags_Borders ) ) {
        const std::vector<std::pair<std::string, std::string>> msgs =
            Messages::recent_messages( colony_log_message_count );
        if( msgs.empty() ) {
            ImGui::TextDisabled( "%s", _( "No messages." ) );
        } else {
            // Newest at the bottom, matching the classic sidebar log.
            for( const auto &msg : msgs ) {
                cataimgui::draw_colored_text( string_format( "%s %s", msg.first, msg.second ),
                                              ImGui::GetContentRegionAvail().x );
            }
            if( ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f ) {
                ImGui::SetScrollHereY( 1.0f );
            }
        }
    }
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::TextUnformatted( _( "Map" ) );
    cataimgui::PushMonoFont();
    overmap_ui::draw_overmap_chunk_imgui( u, cam_omt, colony_overmap_w, colony_overmap_h );
    ImGui::PopFont();
}

cataimgui::bounds colony_sidebar_ui::get_bounds()
{
    const ImVec2 display = ImGui::GetMainViewport()->Size;
    const bool on_right = get_option<std::string>( "SIDEBAR_POSITION" ) != "left";
    // Prefer the reserved panel width so ImGui aligns with terrain clipping.
    int chars = on_right ? panel_manager::get_manager().get_width_right()
                : panel_manager::get_manager().get_width_left();
    if( chars <= 0 ) {
        chars = colony_sidebar_chars;
    }
    const float w = static_cast<float>( str_width_to_pixels( chars ) );
    return { on_right ? display.x - w : 0.f, 0.f, w, display.y };
}

std::unique_ptr<colony_sidebar_ui> g_colony_sidebar;

} // namespace

void ensure_colony_imgui_sidebar()
{
    if( !is_colony_god_mode() ) {
        shutdown_colony_imgui_sidebar();
        return;
    }
    if( !g_colony_sidebar ) {
        g_colony_sidebar = std::make_unique<colony_sidebar_ui>();
    }
}

void shutdown_colony_imgui_sidebar()
{
    g_colony_sidebar.reset();
}

bool colony_imgui_sidebar_active()
{
    return static_cast<bool>( g_colony_sidebar );
}
