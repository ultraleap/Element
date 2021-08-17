using System.Numerics;
using ImGuiNET;

namespace ElementImGui
{
    public static class Splitter
    {
        public static void DrawHorizontal(float thickness, ref float height1, ref float height2, float minHeight1, float minHeight2, float dragWidth = -1f)
            => Draw(true, thickness, ref height1, ref height2, minHeight1, minHeight2, dragWidth);
        
        public static void DrawVertical(float thickness, ref float width1, ref float width2, float minWidth1, float minWidth2, float dragHeight = -1f)
            => Draw(true, thickness, ref width1, ref width2, minWidth1, minWidth2, dragHeight);
        
        private static void Draw(bool vertical, float thickness, ref float size0, ref float size1,
            float minSize0, float minSize1, float size = -1.0f)
        {
            var backupPos = ImGui.GetCursorPos();

            if (vertical)
                ImGui.SetCursorPosY(backupPos.Y + size0);
            else
                ImGui.SetCursorPosX(backupPos.X + size0);

            /*ImGui.PushStyleColor(ImGuiCol.Button, Vector4.Zero);
            ImGui.PushStyleColor(ImGuiCol.ButtonActive, Vector4.Zero);
            ImGui.PushStyleColor(ImGuiCol.ButtonHovered, new Vector4(0.6f,0.6f,0.6f,0.1f));*/

            ImGui.Button("##Splitter", new Vector2(vertical ? thickness : size, !vertical ? thickness : size));
            //ImGui.PopStyleColor(3);

            ImGui.SetItemAllowOverlap(); // This is to allow having other buttons OVER our splitter. 

            if (ImGui.IsItemActive())
            {
                var mouseDelta = vertical ? ImGui.GetMouseDragDelta().Y : ImGui.GetMouseDragDelta().X;

                // Minimum pane size
                if (mouseDelta < minSize0 - size0)
                    mouseDelta = minSize0 - size0;
                if (mouseDelta > size1 - minSize1)
                    mouseDelta = size1 - minSize1;

                // Apply resize
                size0 += mouseDelta;
                size1 -= mouseDelta;
            }
            ImGui.SetCursorPos(backupPos);
        }
    }
}