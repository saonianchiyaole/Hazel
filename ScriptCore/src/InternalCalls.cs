﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Hazel
{

    static class InternalCalls
    {


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Test();

       

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode keycode);

        #region Transform
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(ulong id, out Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(ulong id, ref Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetRotation(ulong id, out Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetRotation(ulong id, ref Vector3 rotation);


        #endregion

        #region Entity
        [MethodImplAttribute(MethodImplOptions.InternalCall)] 
        internal extern static bool Entity_HasComponent(ulong id, Type componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_FindEntityByName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(ulong id);
        #endregion


        #region Animation

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationComponent_Play(ulong id, string animationName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AnimationComponent_Stop(ulong id);

        #endregion
    }
}

