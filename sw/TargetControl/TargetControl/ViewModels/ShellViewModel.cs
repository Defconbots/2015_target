using System;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

using Caliburn.Micro;

using MahApps.Metro;
using MahApps.Metro.Controls;

using TargetControl.Models;
using System.Windows.Input;
using System.Collections.ObjectModel;

namespace TargetControl
{
    public sealed class ShellViewModel : Conductor<IMainScreenTabItem>.Collection.OneActive
    {
        public BindableCollection<Team> Teams { get; set; }
        public BindableCollection<IMainScreenTabItem> Tabs { get; set; }

        public ShellViewModel(IEnumerable<IMainScreenTabItem> tabs, IEventAggregator eventAggregator)
        {
            DisplayName = "DEFCONBOTS CONTEST CONTROLLER";

            Tabs = new BindableCollection<IMainScreenTabItem>();

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

            foreach (var tab in tabs)
            {
                Tabs.Add(tab);
            }
        }
    }

    public interface IMainScreenTabItem
    {
    }
}
