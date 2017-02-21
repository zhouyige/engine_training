using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SteeringBehavior
{
    public partial class Form1 : Form
    {

        private MoveNpc npc1;
        private MoveNpc npc2;

        public Form1()
        {
            InitializeComponent();
            npc1 = new MoveNpc();
            npc1.init();
            npc2 = new MoveNpc();
            npc2.init();
        }

        private void panel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void Form1_Paint(object sender, PaintEventArgs e)
        {
        }

        private void button1_Click(object sender, EventArgs e)
        {

            timer1.Stop();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {         
            npc1.flee(npc2.Loc);
            npc1.update(1.0f);
            panel1.Location = new Point(npc1.Loc.X, npc1.Loc.Y);
            panel2.Location = new Point(npc2.Loc.X, npc2.Loc.Y);
            this.Invalidate();
        }

        private bool mosedown = false;
        private void panel1_MouseDown(object sender, MouseEventArgs e)
        {
            // panel1.BackColor = Color.DarkOrange;
            mosedown = true;

        }

        private void panel1_MouseMove(object sender, MouseEventArgs e)
        {
           if (mosedown == true)
           {
                panel1.Location = new Point(e.Location.X, e.Location.Y);
           }
           
        }

        private void panel1_MouseUp(object sender, MouseEventArgs e)
        {
            mosedown = false;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            npc2.Loc = new Point(100, 100);
            npc1.Loc = new Point(150, 150);
            npc1.v.x = 20.0f;
            npc1.v.y = 10.5f;
            npc1.a.x = 4.0f;
            npc1.a.y = 3.0f;
        }
    }
}
