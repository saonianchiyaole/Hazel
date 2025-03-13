using System;
using System.Numerics;
using System.Runtime;
using System.Runtime.CompilerServices;

namespace Hazel
{
    public class Player : Entity
    {
        private Vector3 forward;
        private Vector3 right;
        public int testValue;
        public float testValue2;
        public float speed = 0.1f;
        private double PI = 3.1415926;

        public void OnCreate()
        {
            Console.WriteLine("HelloWorld!");
            testValue = 1;
            speed = 0.1f;
            Hazel.InternalCalls.Test();
            forward = new Vector3(0.0f, 0.0f, 1.0f);
            right = new Vector3(1.0f, 0.0f, 0.0f);
        }


        public void OnUpdate(float ts)
        {
           
            Vector3 rotation = GetComponent<TransformComponent>().rotation;



            //Translate radians into degree
           /* Quaternion quaternion = Quaternion.CreateFromYawPitchRoll(rotation.x, rotation.y, rotation.z);
            Matrix4x4 rotationMatrix = Matrix4x4.CreateFromQuaternion(quaternion);
            forward = new Vector3(rotationMatrix.M13, rotationMatrix.M23, rotationMatrix.M33);
            right = new Vector3(rotationMatrix.M11, rotationMatrix.M21, rotationMatrix.M31);*/

            Vector3 velocity = new Vector3(0.0f, 0.0f, 0.0f);
            bool isRunning = false;

            if (Input.IsKeyDown(KeyCode.W))
            {
                velocity.z = speed * 0.5f * -1.0f;
                GetComponent<AnimationComponent>().Play("Alien_Run");
                isRunning = true;
            }
            if (Input.IsKeyDown(KeyCode.A))
            {
                velocity.x += speed * 0.5f * -1.0f;
                GetComponent<AnimationComponent>().Play("Alien_Run");
                isRunning = true;
            }
            if (Input.IsKeyDown(KeyCode.S))
            {
                velocity.z = (speed * 0.5f) ;
                GetComponent<AnimationComponent>().Play("Alien_Run");
                isRunning = true;
            }
            if (Input.IsKeyDown(KeyCode.D))
            {
                velocity.x += (0.5f * speed) ;
                GetComponent<AnimationComponent>().Play("Alien_Run");
                isRunning = true;
            }
            if (Input.IsKeyDown(KeyCode.Space))
            {
                //velocity.x += (0.5f * speed);
                GetComponent<AnimationComponent>().Play("Alien_Jump");
                isRunning = true;
            }
            if (!isRunning)
                GetComponent<AnimationComponent>().Play("Alien_Idle");

            if (velocity.x != 0.0f || velocity.y != 0.0f || velocity.z != 0.0f)
            {

                Vector3 newTranslation = GetComponent<TransformComponent>().translation;
                newTranslation += velocity;
                
                GetComponent<TransformComponent>().translation = newTranslation;
            }

        }


    }


}