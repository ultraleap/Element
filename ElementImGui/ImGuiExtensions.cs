using System.Collections.Generic;
using ImGuiNET;
using ResultNET;

namespace ElementImGui
{
    public static class ImGuiExtensions
    {
        public static void DisplayAsImGuiText(this IEnumerable<ResultMessage> messages)
        {
            foreach (var msg in messages)
            {
                ImGui.Text(msg.ToString());
            }
        }
    }
}