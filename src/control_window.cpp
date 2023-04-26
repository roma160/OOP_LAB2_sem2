#include "control_window.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#include "vec2.h"

void display_control_window(Field& field) {
    ImGui::Begin("Control window", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowSize({300, 500}, ImGuiCond_Appearing);

    if(ImGui::BeginTable("Graph on field: ", 2, 
        ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | 
        ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody)
    ) {
        ImGui::TableSetupColumn("Graphs", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("");
        ImGui::TableHeadersRow();

        const auto& graphs = field.get_graphs();
        for(int g = 0; g < graphs.size(); g++){
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            const int n = graphs[g].connections.size();
            const int m = graphs[g].edges.size();

            const static ImGuiTreeNodeFlags tree_node_flags = 
                ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

            typedef Field::FGraphLink l;
            static l hovered(-1, 0);

            if(ImGui::TreeNodeEx((void*) (intptr_t) g, tree_node_flags, "%d Graph[%d]", g, n)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Nodes");
                for(int i = 0; i < n; i++){
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TreeNodeEx(
                        (void*) (intptr_t) i,
                        tree_node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
                        "%d Node", i
                    );
                    if(ImGui::IsItemHovered() && hovered.graph_id == -1){
                        hovered = l{g, i};
                        field.select_point(i, g);
                    }
                    else if(!ImGui::IsItemHovered() && hovered == l{g, i}) {
                        field.disselect_point(i, g);
                        hovered.graph_id = -1;
                    }
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Edges");
                for(int i = 0; i < m; i++){
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    //ImGui::InputText(,)
                    if(ImGui::IsItemHovered() && hovered.graph_id == -1){
                        hovered = l{g, i};
                        field.select_point(i, g);
                    }
                    else if(!ImGui::IsItemHovered() && hovered == l{g, i}) {
                        field.disselect_point(i, g);
                        hovered.graph_id = -1;
                    }
                }

                ImGui::TreePop();
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}