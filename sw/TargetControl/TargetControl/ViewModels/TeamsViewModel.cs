using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Caliburn.Micro;

namespace TargetControl
{
    public sealed class TeamsViewModel : Screen, IMainScreenTabItem
    {
        public TeamsViewModel()
        {
            DisplayName = "Teams";
        }

        public void AddTeam()
        {
        }
    }
}
