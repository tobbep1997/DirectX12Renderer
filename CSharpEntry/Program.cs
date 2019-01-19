using System.Diagnostics;
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
            
            window.Create(Process.GetCurrentProcess(),
                "Test", 
                1280, 
                720, 
                false);
            


            renderingManager.Init(window);


            while (window.IsOpen)
            {
                if (window.Updating())
                {
                    //Window is updating 
                }

                renderingManager.Flush(camera);
            }
            renderingManager.Release();
        }

    }
}
