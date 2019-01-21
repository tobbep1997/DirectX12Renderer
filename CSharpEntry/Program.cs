using System.Diagnostics;
using DXMath;
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

            DXDeltaTime deltaTimer = new DXDeltaTime();

            window.Create(Process.GetCurrentProcess(),
                "Test", 
                1280, 
                720, 
                false);
            
            renderingManager.Init(window);

            DXStaticMesh cubeMesh = new DXStaticMesh();

            DXTexture texture = new DXTexture();
            DXTexture normal = new DXTexture();
            DXTexture metallic = new DXTexture();
            DXTexture displacement = new DXTexture();

            DXDrawable drawable = new DXDrawable();
            
            DXDirectionalLight  directionalLight = new DXDirectionalLight(renderingManager, window);
            directionalLight.Init();


            directionalLight.Intensity = 10;
            directionalLight.SetDirection(new DXVector(0, -1, -1, 0));
            directionalLight.Update();
            
            bool loadSucceeded = true;

            if (cubeMesh.Init())
                if (cubeMesh.LoadStaticMesh("../../Models/Cube.fbx"))
                    if (!cubeMesh.CreateBuffer(renderingManager))
                        loadSucceeded = false;

            if (!texture.LoadDDSTexture("../../Texture/Brick/Brick_diffuse.DDS", true, renderingManager))
                loadSucceeded = false;
            if (!normal.LoadDDSTexture("../../Texture/Brick/Brick_normal.DDS", true, renderingManager))
                loadSucceeded = false;
            if (!metallic.LoadDDSTexture("../../Texture/Brick/Brick_metallic.DDS", true, renderingManager))
                loadSucceeded = false;
            if (!displacement.LoadDDSTexture("../../Texture/Brick/Brick_height.DDS", true, renderingManager))
                loadSucceeded = false;

            drawable.SetStaticMesh(cubeMesh);
            drawable.SetTexture(texture);
            drawable.SetNormalMap(normal);
            drawable.SetMetallicMap(metallic);
            drawable.SetDisplacementMap(displacement);
            drawable.SetPosition(new DXVector(0,0,-5, 1));
            drawable.Update();

            camera.SetPosition(new DXVector(0, 0, 0, 1));
            camera.SetDirection(new DXVector(0, 0, -1, 0));
            camera.Update();

            deltaTimer.Init();
            while (window.IsOpen && loadSucceeded)
            {
                float deltaTime = deltaTimer.GetDeltaTime();
                if (window.Updating())
                {
                    //Window is updating 
                }

                MoveCamera(ref camera, deltaTime);

                if (DXInput.IsKeyPressed('Q'))
                {
                    directionalLight.Queue();
                }

                drawable.Draw(renderingManager);
                renderingManager.Flush(camera);
            }
            texture.Release();
            normal.Release();
            metallic.Release();
            displacement.Release();
            cubeMesh.Release();

            directionalLight.Release();
            

            renderingManager.Release();
        }

        static void MoveCamera(ref DXCamera camera, float deltaTime)
        {
            float sprintMod = 0.5f;
            if (DXInput.IsKeyPressed(16))
                sprintMod = 4.0f;
            float moveSpeed = 2.0f * sprintMod;
            float rotSpeed = 2.0f;

            if (DXInput.IsKeyPressed('A'))
                camera.Translate(new DXVector(-moveSpeed * deltaTime, 0, 0, 1));
            else if (DXInput.IsKeyPressed('D'))
                camera.Translate(new DXVector(moveSpeed * deltaTime, 0, 0, 1));

            if (DXInput.IsKeyPressed('W'))
                camera.Translate(new DXVector(0, 0, moveSpeed * deltaTime, 1));
            else if (DXInput.IsKeyPressed('S'))
                camera.Translate(new DXVector(0, 0, -moveSpeed * deltaTime, 1));

            if (DXInput.IsKeyPressed(37))
                camera.Rotate(new DXVector(0, -rotSpeed * deltaTime, 0, 0));
            else if (DXInput.IsKeyPressed(39))
                camera.Rotate(new DXVector(0, rotSpeed * deltaTime, 0, 0));

            if (DXInput.IsKeyPressed(38))
                camera.Rotate(new DXVector(-rotSpeed * deltaTime, 0, 0, 0));
            else if (DXInput.IsKeyPressed(40))
                camera.Rotate(new DXVector(rotSpeed * deltaTime, 0, 0, 0));
            camera.Update();
            
        }

    }
}
