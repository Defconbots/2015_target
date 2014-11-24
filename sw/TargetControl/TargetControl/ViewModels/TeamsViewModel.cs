using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Caliburn.Micro;

using TargetControl.Models;

namespace TargetControl
{
    public sealed class TeamsViewModel : Screen, IMainScreenTabItem
    {
        public BindableCollection<Team> Teams { get; set; }

        public TeamsViewModel()
        {
            DisplayName = "Teams";

            Teams = new BindableCollection<Team>
            {
                new Team{Name="test1",HitId="1",
                    Members = new List<TeamMember>{new TeamMember{Name = "Joe",Email="joe@joe.com",Website="joe.com",PhoneNumber="555-555-5555"}},
                    QualScores = new List<int>{0,1,2,3},
                    FinalScores = new List<int>{5,6,7,8}},
                new Team{Name="test1",HitId="1",
                    Members = new List<TeamMember>{new TeamMember{Name = "Joe",Email="joe@joe.com",Website="joe.com",PhoneNumber="555-555-5555"},
                                                   new TeamMember{Name = "Bob",Email="bob@bob.com",Website="bob.com",PhoneNumber="555-555-5551"}},
                    QualScores = new List<int>{0,1,2,3},
                    FinalScores = new List<int>{5,6,7,8}},
                new Team{Name="test1",HitId="1",
                    Members = new List<TeamMember>{new TeamMember{Name = "Joe",Email="joe@joe.com",Website="joe.com",PhoneNumber="555-555-5555"}},
                    QualScores = new List<int>{0,1,2,3},
                    FinalScores = new List<int>{5,6}}
            };
        }

        public void AddTeam()
        {
        }
    }
}
