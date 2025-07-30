#include "../nfc_app_i.h"

#include "../helpers/protocol_support/nfc_protocol_support_gui_common.h"
#include "../helpers/protocol_support/felica/felica_render.h"

enum {
    FelicaMoreInfoStateMenu,
    FelicaMoreInfoStateItem, // MUST be last, states >= this correspond with submenu index
};

enum SubmenuIndex {
    SubmenuIndexDirectory,
    SubmenuIndexDynamic, // dynamic indices start here
};

void nfc_scene_felica_more_info_on_enter(void* context) {
    NfcApp* nfc = context;
    Submenu* submenu = nfc->submenu;

    const uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneFelicaMoreInfo);
    const FelicaData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolFelica);

    submenu_add_item(
        submenu,
        "Directory",
        SubmenuIndexDirectory,
        nfc_protocol_support_common_submenu_callback,
        nfc);

    FuriString* label = furi_string_alloc();

    switch(data->ic_type) {
    case FelicaLite:
        furi_string_printf(label, "All blocks");
        submenu_add_item(
            submenu,
            furi_string_get_cstr(label),
            SubmenuIndexDynamic,
            nfc_protocol_support_common_submenu_callback,
            nfc);
        break;
    case FelicaStandard:
        for(uint32_t i = 0; i < simple_array_get_count(data->services); ++i) {
            const FelicaService* service = simple_array_cget(data->services, i);
            bool is_public = (service->attr & FELICA_SERVICE_ATTRIBUTE_UNAUTH_READ) == 1;
            if(!is_public) {
                continue;
            }
            furi_string_printf(label, "Readable serv %04X", service->code);
            submenu_add_item(
                submenu,
                furi_string_get_cstr(label),
                i + SubmenuIndexDynamic,
                nfc_protocol_support_common_submenu_callback,
                nfc);
        }
        break;
    default:
        break;
    }

    furi_string_free(label);

    if(state >= FelicaMoreInfoStateItem) {
        submenu_set_selected_item(
            nfc->submenu, state - FelicaMoreInfoStateItem + SubmenuIndexDynamic);
        scene_manager_set_scene_state(
            nfc->scene_manager, NfcSceneFelicaMoreInfo, FelicaMoreInfoStateMenu);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_felica_more_info_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    bool consumed = false;

    const uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneFelicaMoreInfo);
    const FelicaData* data = nfc_device_get_data(nfc->nfc_device, NfcProtocolFelica);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexDirectory) {
            FuriString* temp_str = furi_string_alloc();
            nfc_more_info_render_felica_dir(data, temp_str);

            widget_add_text_scroll_element(
                nfc->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));
            furi_string_free(temp_str);

            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
            scene_manager_set_scene_state(
                nfc->scene_manager,
                NfcSceneFelicaMoreInfo,
                FelicaMoreInfoStateItem + SubmenuIndexDirectory);
            consumed = true;
        } else {
            const uint16_t service_ind = event.event - 1; // offset the three enums above
            FuriString* temp_str = furi_string_alloc();

            switch(data->ic_type) {
            case FelicaLite:
                nfc_more_info_render_felica_lite_dump(data, temp_str);
                break;
            case FelicaStandard:
                const FelicaService* service = simple_array_cget(data->services, service_ind);
                furi_string_cat_printf(temp_str, "\e#Service %04X\n", service->code);
                nfc_more_info_render_felica_blocks(data, temp_str, service->code);
                break;
            default:
                furi_string_printf(
                    temp_str,
                    "IC type %d for Felica protocol\nNot implemented yet.",
                    data->ic_type);
                break;
            }
            widget_add_text_scroll_element(
                nfc->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));
            furi_string_free(temp_str);

            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneFelicaMoreInfo, FelicaMoreInfoStateItem + event.event);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(state >= FelicaMoreInfoStateItem) {
            widget_reset(nfc->widget);
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneFelicaMoreInfo, FelicaMoreInfoStateMenu);
        } else {
            widget_reset(nfc->widget);
            // Return directly to the Info scene
            scene_manager_search_and_switch_to_previous_scene(nfc->scene_manager, NfcSceneInfo);
        }
        consumed = true;
    }

    return consumed;
}

void nfc_scene_felica_more_info_on_exit(void* context) {
    NfcApp* nfc = context;

    // Clear views
    widget_reset(nfc->widget);
    submenu_reset(nfc->submenu);
}
