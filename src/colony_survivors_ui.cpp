#include "colony_survivors_ui.h"

#include <algorithm>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include <imgui/imgui.h>

#include "avatar.h"
#include "basecamp.h"
#include "cata_imgui.h"
#include "character_id.h"
#include "colony_camera.h"
#include "color.h"
#include "coordinates.h"
#include "display.h"
#include "faction.h"
#include "game.h"
#include "game_constants.h"
#include "input_context.h"
#include "localized_comparator.h"
#include "map.h"
#include "npc.h"
#include "output.h"
#include "overmapbuffer.h"
#include "player_activity.h"
#include "point.h"
#include "string_formatter.h"
#include "translations.h"
#include "ui_manager.h"
#include "memory_fast.h"

namespace
{

std::string survivor_activity_summary( const npc &guy )
{
    if( guy.current_target() != nullptr ) {
        return _( "In combat" );
    }
    if( guy.has_companion_mission() ) {
        return guy.get_current_activity();
    }
    if( !guy.activity.is_null() ) {
        return string_format( "%s", guy.activity.get_verb() );
    }
    if( guy.has_job() ) {
        activity_id top;
        int top_pri = 0;
        for( const activity_id &elem : guy.job.get_prioritised_vector() ) {
            const int pri = guy.job.get_priority_of_job( elem );
            if( pri > top_pri ) {
                top_pri = pri;
                top = elem;
            }
        }
        if( !top.is_null() ) {
            player_activity sample( top );
            return string_format( _( "%s (pri %d)" ), sample.get_verb(), top_pri );
        }
    }
    return guy.get_current_status();
}

std::vector<npc *> collect_colony_survivors()
{
    std::vector<npc *> out;
    std::set<character_id> seen;

    Character &player_character = get_player_character();
    g->validate_camps();
    if( player_character.get_faction() ) {
        for( const tripoint_abs_omt &elem : player_character.camps ) {
            std::optional<basecamp *> found = overmap_buffer.find_camp( elem.xy() );
            if( !found || ( *found )->get_owner() != player_character.get_faction()->id ) {
                continue;
            }
            for( const npc_ptr &guy : ( *found )->get_npcs_assigned() ) {
                if( !guy || !seen.insert( guy->getID() ).second ) {
                    continue;
                }
                out.push_back( guy.get() );
            }
        }
    }

    std::vector<npc *> followers;
    overmap_buffer.populate_followers_vec( followers );
    for( npc *guy : followers ) {
        if( !guy || !seen.insert( guy->getID() ).second ) {
            continue;
        }
        out.push_back( guy );
    }

    std::sort( out.begin(), out.end(), []( const npc * a, const npc * b ) {
        return localized_compare( a->disp_name(), b->disp_name() );
    } );
    return out;
}

class colony_survivors_ui_impl : public cataimgui::window
{
    public:
        std::string last_action;
        explicit colony_survivors_ui_impl() : cataimgui::window( _( "Colony survivors" ),
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav ) {
        }

        int selected = 0;
        bool focus_requested = false;

    private:
        float window_width = std::clamp( float( str_width_to_pixels( EVEN_MINIMUM_TERM_WIDTH ) ),
                                         ImGui::GetMainViewport()->Size.x / 2,
                                         ImGui::GetMainViewport()->Size.x );
        float window_height = std::clamp( float( str_height_to_pixels( EVEN_MINIMUM_TERM_HEIGHT ) ),
                                          ImGui::GetMainViewport()->Size.y / 2,
                                          ImGui::GetMainViewport()->Size.y );

    protected:
        void draw_controls() override;
        cataimgui::bounds get_bounds() override {
            return { -1.f, -1.f, window_width, window_height };
        }
};

void draw_survivor_detail( npc *guy )
{
    const float col_width = ImGui::GetContentRegionAvail().x;
    if( !guy ) {
        cataimgui::draw_colored_text( _( "No survivors in your colony yet." ), c_red, col_width );
        return;
    }

    cataimgui::draw_colored_text( guy->disp_name(), c_white, col_width );
    cataimgui::draw_colored_text( string_format( _( "Status: %s" ), survivor_activity_summary( *guy ) ),
                                  c_light_gray, col_width );

    const tripoint_abs_omt omt = guy->pos_abs_omt();
    cataimgui::draw_colored_text( string_format( _( "Location: %s" ), omt.to_string() ),
                                  c_light_gray, col_width );

    if( guy->assigned_camp ) {
        cataimgui::draw_colored_text( string_format( _( "Assigned camp: %s" ),
                                      guy->assigned_camp->to_string() ),
                                      c_light_gray, col_width );
    }

    const auto &[hp_desc, hp_color] = guy->hp_description();
    cataimgui::draw_colored_text( string_format( _( "Condition: %s" ), hp_desc ), hp_color, col_width );

    const auto &[hunger, hunger_color] = display::hunger_text_color( *guy );
    const auto &[thirst, thirst_color] = display::thirst_text_color( *guy );
    const auto &[sleepiness, sleepiness_color] = display::sleepiness_text_color( *guy );
    const std::string nominal = pgettext( "needs", "Nominal" );
    cataimgui::draw_colored_text( string_format( _( "Hunger: %s" ),
                                  hunger.empty() ? nominal : hunger ), hunger_color, col_width );
    cataimgui::draw_colored_text( string_format( _( "Thirst: %s" ),
                                  thirst.empty() ? nominal : thirst ), thirst_color, col_width );
    cataimgui::draw_colored_text( string_format( _( "Sleepiness: %s" ),
                                  sleepiness.empty() ? nominal : sleepiness ),
                                  sleepiness_color, col_width );

    cataimgui::draw_colored_text( string_format( _( "Wielding: %s" ), guy->weapname_simple() ),
                                  c_light_gray, col_width );

    input_context ctxt( "COLONY_SURVIVORS" );
    ctxt.register_action( "CONFIRM" );
    cataimgui::draw_colored_text(
        string_format( _( "Press [%s] to center the camera on this survivor (no control)." ),
                       ctxt.get_desc( "CONFIRM" ) ),
        c_dark_gray, col_width );
}

void colony_survivors_ui_impl::draw_controls()
{
    std::vector<npc *> survivors = collect_colony_survivors();
    if( survivors.empty() ) {
        selected = 0;
    } else if( selected < 0 ) {
        selected = static_cast<int>( survivors.size() ) - 1;
    } else if( selected >= static_cast<int>( survivors.size() ) ) {
        selected = 0;
    }

    if( last_action == "UP" ) {
        selected--;
        if( selected < 0 && !survivors.empty() ) {
            selected = static_cast<int>( survivors.size() ) - 1;
        }
    } else if( last_action == "DOWN" ) {
        selected++;
        if( !survivors.empty() && selected >= static_cast<int>( survivors.size() ) ) {
            selected = 0;
        }
    } else if( last_action == "CONFIRM" && !survivors.empty() ) {
        focus_requested = true;
    }

    npc *picked = survivors.empty() ? nullptr : survivors[selected];

    if( ImGui::BeginTable( "##survivors", 2, ImGuiTableFlags_None,
                           ImGui::GetContentRegionAvail() ) ) {
        ImGui::TableSetupColumn( "list", ImGuiTableColumnFlags_WidthStretch, 0.4f );
        ImGui::TableSetupColumn( "detail", ImGuiTableColumnFlags_WidthStretch, 0.6f );
        ImGui::TableNextColumn();
        if( ImGui::BeginChild( "##list", ImVec2( 0, 0 ), ImGuiChildFlags_Borders ) ) {
            if( ImGui::BeginListBox( "##LISTBOX", ImGui::GetContentRegionAvail() ) ) {
                for( size_t i = 0; i < survivors.size(); i++ ) {
                    npc *guy = survivors[i];
                    const bool is_selected = static_cast<int>( i ) == selected;
                    ImGui::PushID( static_cast<int>( i ) );
                    if( ImGui::Selectable( "", is_selected ) ) {
                        selected = static_cast<int>( i );
                    }
                    if( is_selected ) {
                        ImGui::SetScrollHereY();
                        ImGui::SetItemDefaultFocus();
                    }
                    ImGui::SameLine();
                    const std::string line = string_format( "%s — %s", guy->disp_name(),
                                                            survivor_activity_summary( *guy ) );
                    cataimgui::draw_colored_text( line, ImGui::GetContentRegionAvail().x );
                    ImGui::PopID();
                }
                ImGui::EndListBox();
            }
            ImGui::EndChild();
        }
        ImGui::TableNextColumn();
        if( ImGui::BeginChild( "##detail", ImVec2( 0, 0 ), ImGuiChildFlags_Borders ) ) {
            draw_survivor_detail( picked );
            ImGui::EndChild();
        }
        ImGui::EndTable();
    }
}

} // namespace

void colony_survivors_ui()
{
    if( !is_colony_god_mode() ) {
        return;
    }

    input_context ctxt( "COLONY_SURVIVORS" );
    colony_survivors_ui_impl ui;

    ctxt.register_navigate_ui_list();
    ctxt.register_action( "CONFIRM", to_translation( "Center camera on survivor" ) );
    ctxt.register_action( "HELP_KEYBINDINGS" );
    ctxt.register_action( "QUIT" );
    ctxt.set_timeout( 10 );

    while( true ) {
        ui_manager::redraw_invalidated();
        ui.last_action = ctxt.handle_input();

        if( ui.focus_requested ) {
            ui.focus_requested = false;
            std::vector<npc *> survivors = collect_colony_survivors();
            if( !survivors.empty() && ui.selected >= 0 &&
                ui.selected < static_cast<int>( survivors.size() ) ) {
                colony_god_focus_on( get_avatar(), survivors[ui.selected]->pos_abs() );
            }
            break;
        }

        if( ui.last_action == "QUIT" || !ui.get_is_open() ) {
            break;
        }
    }
}
