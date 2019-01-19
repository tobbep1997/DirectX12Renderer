using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ID3D12;

namespace CSharpEntry
{
    class Program //NOLINT
    {
        static void Main(string[] args)
        {
            DXWindow window = new DXWindow();
            DXRenderingManager renderingManager = new DXRenderingManager();

            DXCamera camera = new DXCamera();

            unsafe
            {
                window.Create(Process.GetCurrentProcess().Handle.ToPointer(),
                    "Test", 
                    1280, 
                    720, 
                    false);

                renderingManager.Init(window);
            }

            while (window.IsOpen)
            {
                while (window.Updating())
                {
                    //Window is updating 
                }

                renderingManager.Flush(camera);
            }
            renderingManager.Release();
        }
    }
}
