using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;

namespace SteeringBehavior
{
    struct Vector3
    {
        public float x;
        public float y;
        public float z;

       
        public void normalize()
        {
            float inlen = 1.0f / (float)Math.Sqrt((double)(x * x + y * y));

            x = inlen * x;
            y = inlen * z;
        }
    }

    class MoveNpc
    {
        public float speedN1 = 0;
        public Point Loc;
        public Vector3 v;
        public Vector3 a;

        public void init()
        {
            Loc.X = 0;
            Loc.Y = 0;
            a.y = 0.0f;
            a.x = 0.0f;
            v.x = 0.0f;
            v.y = 0.0f;
        }

        public void update(float t)
        {
            Loc.X = Loc.X + (int)(v.x * t + 0.5f * t * t * a.x);
            Loc.Y = Loc.Y + (int)(v.y * t + 0.5f * t * t * a.y);
        }

        
        public void flee(Point pos)
        {
            Vector3 vf;
            vf.z = 0.0f;
            vf.x =  Loc.X - pos.X;
            vf.y = Loc.Y - pos.Y;
            vf.normalize();
            a.x = -vf.x*10 - v.x;
            a.y = -vf.y*10 - v.y;

        }

        public void seek(Point pos)
        {

        }
        
        public void arrive(Point pos)
        {

        }

        public void avoid(Point pos, int r)
        {

        }
    }
}
