using System;
using System.Collections.Generic;
using System.Linq;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.GamerServices;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework.Media;
using Microsoft.Xna.Framework.Net;
using Microsoft.Xna.Framework.Storage;

namespace IrvingParkRoad
{
    /// <summary>
    /// This is the main type for your game
    /// </summary>
    public class IPR : Microsoft.Xna.Framework.Game
    {
        GraphicsDeviceManager graphics;
        SpriteBatch spriteBatch;
        Quad screenQuad;
        Quad spriteQuad;
        Quad spriteQuadOffset;

        float lightTravelerx = 1.0f;
        float lightTravelery = 0.0f;

        Matrix viewMatrix, projectionMatrix;
        Texture2D sprite;
        Texture2D heightmap;

        Effect clearBuffer;
        Effect generateMaps;
        Effect calculateLights;
        Effect renderBuffers;

        VertexDeclaration screenVertexDecl;
        VertexDeclaration spriteVertexDecl;
        VertexDeclaration spriteOffsetVertexDecl;


        RenderTarget2D layer0;          //Background Deep
        RenderTarget2D layer1;          //Background Shallow
        RenderTarget2D layer2;          //Frontground
        RenderTarget2D colorRT;       //Main Layer
        RenderTarget2D heightRT;//Lighting Occluder
        RenderTarget2D lightRT;          //LightingMap //RGBA - RG = Vector B = Height A = intensity
        RenderTarget2D finalRT;

        public IPR()
        {
            graphics = new GraphicsDeviceManager(this);
            

            graphics.PreferredBackBufferWidth = 1280;
            graphics.PreferredBackBufferHeight = 720;

            graphics.MinimumVertexShaderProfile = ShaderProfile.VS_3_0;
            graphics.MinimumPixelShaderProfile = ShaderProfile.PS_3_0;

            Content.RootDirectory = "Content";
            //Components.Add(new FrameRateCounter(this));
        }


        /// <summary>
        /// Allows the game to perform any initialization it needs to before starting to run.
        /// This is where it can query for any required services and load any non-graphic
        /// related content.  Calling base.Initialize will enumerate through any components
        /// and initialize them as well.
        /// </summary>
        protected override void Initialize()
        {          
            viewMatrix = Matrix.CreateLookAt(new Vector3(0, 0, 2), Vector3.Zero, Vector3.Up);
            projectionMatrix = Matrix.CreateOrthographic(graphics.PreferredBackBufferWidth / 2.0f, graphics.PreferredBackBufferHeight / 2.0f, 0.0f, 10.0f);

            base.Initialize();

            int bbWidth = GraphicsDevice.PresentationParameters.BackBufferWidth;
            int bbHeight = GraphicsDevice.PresentationParameters.BackBufferHeight;

            colorRT = new RenderTarget2D(GraphicsDevice, bbWidth, bbHeight, 0, SurfaceFormat.Color, RenderTargetUsage.DiscardContents);
            heightRT = new RenderTarget2D(GraphicsDevice, bbWidth, bbHeight, 0, SurfaceFormat.Color, RenderTargetUsage.DiscardContents);
            lightRT = new RenderTarget2D(GraphicsDevice, bbWidth, bbHeight, 0, SurfaceFormat.Color, RenderTargetUsage.DiscardContents);
            finalRT = new RenderTarget2D(GraphicsDevice, bbWidth, bbHeight, 0, SurfaceFormat.Color, RenderTargetUsage.DiscardContents);

            screenVertexDecl = new VertexDeclaration(GraphicsDevice, VertexPositionNormalTexture.VertexElements);
            spriteVertexDecl = new VertexDeclaration(GraphicsDevice, VertexPositionNormalTexture.VertexElements);
            spriteOffsetVertexDecl = new VertexDeclaration(GraphicsDevice, VertexPositionNormalTexture.VertexElements);

            screenQuad = new Quad(Vector3.Zero, Vector3.Backward, Vector3.Up, bbWidth, bbHeight);
            spriteQuad = new Quad(Vector3.Zero, Vector3.Backward, Vector3.Up, 164, 164);
            spriteQuadOffset = new Quad(new Vector3(30, 30, 0), Vector3.Backward, Vector3.Up, 164, 164);
        }
       
        /// <summary>
        /// LoadContent will be called once per game and is the place to load
        /// all of your content.
        /// </summary>
        protected override void LoadContent()
        {
            // Create a new SpriteBatch, which can be used to draw textures.
            spriteBatch = new SpriteBatch(GraphicsDevice);

            sprite = Content.Load<Texture2D>("DerekYu");
            heightmap = Content.Load<Texture2D>("DerekYuMap");

            clearBuffer = Content.Load<Effect>("ClearBuffer");
            generateMaps = Content.Load<Effect>("GenerateMaps");
            calculateLights = Content.Load<Effect>("LightCalc");
            renderBuffers = Content.Load<Effect>("RenderBuffer");
        }

        /// <summary>
        /// UnloadContent will be called once per game and is the place to unload
        /// all content.
        /// </summary>
        protected override void UnloadContent()
        {
        }

        /// <summary>
        /// Allows the game to run logic such as updating the world,
        /// checking for collisions, gathering input, and playing audio.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Update(GameTime gameTime)
        {
            // Allows the game to exit
            if (GamePad.GetState(PlayerIndex.One).Buttons.Back == ButtonState.Pressed)
                this.Exit();

            if (lightTravelerx < GraphicsDevice.PresentationParameters.BackBufferWidth - 1)
            {
                lightTravelerx += 2.5f;
                lightTravelery += 0.5f;
            }
            else
            {
                lightTravelerx = 0.0f;
            }

            if (lightTravelerx > GraphicsDevice.PresentationParameters.BackBufferWidth &&
                lightTravelery > GraphicsDevice.PresentationParameters.BackBufferHeight * 2.2f)
            {
                lightTravelerx = lightTravelery = 0.0f;
            }
            base.Update(gameTime);
        }

        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Draw(GameTime gameTime)
        {
            int Width = graphics.PreferredBackBufferWidth;
            int Height = graphics.PreferredBackBufferHeight;

            //Setup Buffers
            GraphicsDevice.SetRenderTarget(0, colorRT);
            GraphicsDevice.SetRenderTarget(1, heightRT);
            GraphicsDevice.SetRenderTarget(2, lightRT);
            GraphicsDevice.SetRenderTarget(3, finalRT);

            //Clear Them
            clearBuffer.Begin();
            clearBuffer.Techniques[0].Passes[0].Begin();
            GraphicsDevice.VertexDeclaration = screenVertexDecl;
            GraphicsDevice.DrawUserIndexedPrimitives<VertexPositionNormalTexture>(PrimitiveType.TriangleList, screenQuad.Vertices, 0, 4, screenQuad.Indexes, 0, 2);
            clearBuffer.Techniques[0].Passes[0].End();
            clearBuffer.End();

            //Setup Render states
            GraphicsDevice.RenderState.AlphaBlendEnable = true;
            GraphicsDevice.RenderState.AlphaBlendOperation = BlendFunction.Add;
            GraphicsDevice.RenderState.DepthBufferEnable = true;
            GraphicsDevice.RenderState.DepthBufferWriteEnable = false;
            //GraphicsDevice.RenderState.CullMode = CullMode.CullCounterClockwiseFace;
            
            //Generate Maps
            generateMaps.Parameters["World"].SetValue(Matrix.Identity);
            generateMaps.Parameters["View"].SetValue(viewMatrix);
            generateMaps.Parameters["Projection"].SetValue(projectionMatrix);
            generateMaps.Parameters["heightMap"].SetValue(heightmap);
            generateMaps.Parameters["colorMap"].SetValue(sprite);
            generateMaps.CurrentTechnique = generateMaps.Techniques["Default"];
            generateMaps.Begin();
            foreach (EffectPass pass in generateMaps.CurrentTechnique.Passes)
            {
                pass.Begin();
                GraphicsDevice.VertexDeclaration = spriteVertexDecl;
                GraphicsDevice.DrawUserIndexedPrimitives<VertexPositionNormalTexture>(PrimitiveType.TriangleList, spriteQuad.Vertices, 0, 4, spriteQuad.Indexes, 0, 2);

                GraphicsDevice.VertexDeclaration = spriteOffsetVertexDecl;
                GraphicsDevice.DrawUserIndexedPrimitives<VertexPositionNormalTexture>(PrimitiveType.TriangleList, spriteQuadOffset.Vertices, 0, 4, spriteQuadOffset.Indexes, 0, 2);
                pass.End();
            }
            generateMaps.End();

            //Resolve Map Buffers
            GraphicsDevice.SetRenderTarget(0, null);
            GraphicsDevice.SetRenderTarget(1, null);
            GraphicsDevice.SetRenderTarget(2, null);
            GraphicsDevice.SetRenderTarget(3, null);

            GraphicsDevice.SetRenderTarget(0, lightRT);

            //Get Textures from Buffers
            Texture2D colormap = colorRT.GetTexture();
            Texture2D heightMap = heightRT.GetTexture();

            //Setup light calculation parameters
            calculateLights.Parameters["World"].SetValue(Matrix.Identity);
            calculateLights.Parameters["View"].SetValue(viewMatrix);
            calculateLights.Parameters["Projection"].SetValue(projectionMatrix);
            calculateLights.Parameters["LightPos"].SetValue(new Vector2(lightTravelerx, lightTravelery));
            calculateLights.Parameters["LightRadius"].SetValue(1800.0f);
            calculateLights.Parameters["screenWidthHeight"].SetValue(new Vector2(Width, Height));
            calculateLights.Parameters["heightMap"].SetValue(heightMap);
            
            //Start Calculating lights
            calculateLights.Begin();
            calculateLights.Techniques[0].Passes[0].Begin();
            GraphicsDevice.VertexDeclaration = screenVertexDecl;
            GraphicsDevice.DrawUserIndexedPrimitives<VertexPositionNormalTexture>(PrimitiveType.TriangleList, screenQuad.Vertices, 0, 4, screenQuad.Indexes, 0, 2);
            calculateLights.Techniques[0].Passes[0].End();
            calculateLights.End();

            //Resolve Light Calc
            GraphicsDevice.SetRenderTarget(0, null);
            Texture2D lightMap = lightRT.GetTexture();

            //Combining it All

            renderBuffers.Parameters["World"].SetValue(Matrix.Identity);
            renderBuffers.Parameters["View"].SetValue(viewMatrix);
            renderBuffers.Parameters["Projection"].SetValue(projectionMatrix);
            renderBuffers.Parameters["screenWidthHeight"].SetValue(new Vector2(Width, Height));
            renderBuffers.Parameters["colorMap"].SetValue(colormap);
            renderBuffers.Parameters["heightMap"].SetValue(heightMap);
            renderBuffers.Parameters["lightMap"].SetValue(lightMap);

            GraphicsDevice.SetRenderTarget(0, finalRT);

            renderBuffers.Begin();
            renderBuffers.Techniques[0].Passes[0].Begin();
            GraphicsDevice.VertexDeclaration = screenVertexDecl;
            GraphicsDevice.DrawUserIndexedPrimitives<VertexPositionNormalTexture>(PrimitiveType.TriangleList, screenQuad.Vertices, 0, 4, screenQuad.Indexes, 0, 2);
            renderBuffers.Techniques[0].Passes[0].End();
            renderBuffers.End();

            GraphicsDevice.SetRenderTarget(0, null);

            Texture2D finalRender = finalRT.GetTexture();



            spriteBatch.Begin(SpriteBlendMode.None);
            spriteBatch.Draw(colormap, new Rectangle(0, 0, Width / 2, Height / 2), Color.White);
            spriteBatch.Draw(heightMap, new Rectangle(Width / 2, 0, Width / 2, Height / 2), Color.White);
            spriteBatch.Draw(lightMap, new Rectangle(0, Height / 2, Width / 2, Height / 2), Color.White);
            spriteBatch.Draw(finalRender, new Rectangle(Width / 2, Height / 2, Width / 2, Height/2), Color.White);// new Rectangle(Width / 2, Height / 2, Width / 2, Height / 2), Color.White);
            spriteBatch.End();
            base.Draw(gameTime);
        }
    }
}
