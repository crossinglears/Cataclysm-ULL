#include "colony_toolbar.h"

#include <memory>
#include <string>

#include <imgui/imgui.h>

#include "cata_imgui.h"
#include "colony_camera.h"
#include "input_context.h"
#include "translations.h"

namespace
{

class colony_toolbar_ui : public cataimgui::window
{
    public:
        colony_toolbar_ui() : cataimgui::window( _( "Colony leadership" ),
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing ) {
            // Stay behind modal leadership panels (faction UI, uilist, etc.).
            force_to_back = true;
        }

    protected:
        void draw_controls() override;
        cataimgui::bounds get_bounds() override;
};

void colony_toolbar_ui::draw_controls()
{
    // Labels include bound hotkeys when configured (DEFAULTMODE).
    input_context ctxt( "DEFAULTMODE" );
    ctxt.register_action( "colony_hub" );
    ctxt.register_action( "colony_survivors" );
    ctxt.register_action( "factions" );
    ctxt.register_action( "colony_construction" );
    ctxt.register_action( "colony_expeditions" );
    ctxt.register_action( "colony_combat" );

    ImGui::TextUnformatted( _( "Leadership" ) );
    ImGui::SameLine();

    action_button( "colony_hub", ctxt.get_button_text( "colony_hub", _( "Hub" ) ) );
    ImGui::SameLine();
    action_button( "colony_survivors",
                   ctxt.get_button_text( "colony_survivors", _( "Survivors" ) ) );
    ImGui::SameLine();
    action_button( "factions", ctxt.get_button_text( "factions", _( "Overview" ) ) );
    ImGui::SameLine();
    action_button( "colony_construction",
                   ctxt.get_button_text( "colony_construction", _( "Build" ) ) );
    ImGui::SameLine();
    action_button( "colony_expeditions",
                   ctxt.get_button_text( "colony_expeditions", _( "Expeditions" ) ) );
    ImGui::SameLine();
    action_button( "colony_combat", ctxt.get_button_text( "colony_combat", _( "Defense" ) ) );
}

cataimgui::bounds colony_toolbar_ui::get_bounds()
{
    // Top-center of the viewport; width/height from AlwaysAutoResize.
    return { -1.f, 4.f, -1.f, -1.f };
}

std::unique_ptr<colony_toolbar_ui> g_colony_toolbar;

} // namespace

void ensure_colony_toolbar()
{
    if( !is_colony_god_mode() ) {
        shutdown_colony_toolbar();
        return;
    }
    if( !g_colony_toolbar ) {
        g_colony_toolbar = std::make_unique<colony_toolbar_ui>();
    }
}

void shutdown_colony_toolbar()
{
    g_colony_toolbar.reset();
}

std::string colony_toolbar_poll_action()
{
    if( !g_colony_toolbar || !g_colony_toolbar->has_button_action() ) {
        return {};
    }
    return g_colony_toolbar->get_button_action();
}
