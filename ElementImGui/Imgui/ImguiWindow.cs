using System;
using System.Numerics;
using Veldrid;
using Veldrid.Sdl2;
using Veldrid.StartupUtilities;

namespace ElementImGui
{
    public class ImGuiWindow : IDisposable
    {
        public Sdl2Window Window => _window;
        public ImGuiRenderer Renderer => _renderer;

        private readonly ImGuiRenderer _renderer;
        private readonly Sdl2Window _window;
        private readonly GraphicsDevice _gd;
        private readonly CommandList _cl;
        private static readonly Vector3 _clearColor = new Vector3(0.45f, 0.55f, 0.6f);

        public ImGuiWindow(string title)
        {
            VeldridStartup.CreateWindowAndGraphicsDevice(new WindowCreateInfo(50, 50, 1280, 720, WindowState.Normal, title),
                new GraphicsDeviceOptions(true, null, true),
                out _window,
                out _gd);
            _window.Resized += () =>
            {
                _gd.MainSwapchain.Resize((uint) _window.Width, (uint) _window.Height);
                _renderer.WindowResized(_window.Width, _window.Height);
            };
            _cl = _gd.ResourceFactory.CreateCommandList();
            _renderer = new ImGuiRenderer(_gd, _gd.MainSwapchain.Framebuffer.OutputDescription, _window.Width, _window.Height);
        }

        public void RunBlocking(Action doGui)
        {
            // Main application loop
            while (_window.Exists)
            {
                InputSnapshot snapshot = _window.PumpEvents();
                if (!_window.Exists)
                {
                    break;
                }

                _renderer.Update(1f / 60f, snapshot); // Feed the input events to our ImGui controller, which passes them through to ImGui.

                doGui();

                _cl.Begin();
                _cl.SetFramebuffer(_gd.MainSwapchain.Framebuffer);
                _cl.ClearColorTarget(0, new RgbaFloat(_clearColor.X, _clearColor.Y, _clearColor.Z, 1f));
                _renderer.Render(_gd, _cl);
                _cl.End();
                _gd.SubmitCommands(_cl);
                _gd.SwapBuffers(_gd.MainSwapchain);
            }
        }

        public void Dispose()
        {
            // Clean up Veldrid resources
            _gd.WaitForIdle();
            _renderer.Dispose();
            _cl.Dispose();
            _gd.Dispose();
        }
    }
}