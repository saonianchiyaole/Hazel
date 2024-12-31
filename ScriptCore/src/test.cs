using System;
using System.Numerics;
using System.Runtime.CompilerServices;


namespace Hazel
{
    public class Player : Entity
    {
        public Vector2 Position;
        public int testValue;
        public float speed = 19.0f;


        public void OnCreate()
        {
            Console.WriteLine("HelloWorld!");
            speed = 19.0f;
            Position = new Vector2(0.5f, 0.5f);
            Hazel.InternalCalls.Test();
        }

        public void OnUpdate(float ts)
        {

            Vector3 velocity = new Vector3(0.0f, 0.0f, 0.0f);

            if (Input.IsKeyDown(KeyCode.W))
            {
                velocity.y = 0.5f * speed;
            }
            if (Input.IsKeyDown(KeyCode.A))
            {
                velocity.x = -0.5f * speed;
            }
            if (Input.IsKeyDown(KeyCode.S))
            {
                velocity.y = -0.5f * speed;
            }
            if (Input.IsKeyDown(KeyCode.D))
            {
                velocity.x = 0.5f * speed;
            }

            if (velocity.x != 0.0f || velocity.y != 0.0f)
            {

                Vector3 newTranslation = GetComponent<TransformComponent>().translation;
                newTranslation.x += velocity.x;
                newTranslation.y += velocity.y;
                GetComponent<TransformComponent>().translation = newTranslation;
            }

        }


    }


}