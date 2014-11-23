using System;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using MahApps.Metro;
using TargetControl.Models;
using System.Windows.Input;
using System.Collections.ObjectModel;

namespace TargetControl
{
    public class MainWindowViewModel : INotifyPropertyChanged
    {
        public ObservableCollection<Team> Teams { get; set; }
        public MainWindowViewModel()
        {
            Teams = new ObservableCollection<Team>
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
        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void RaisePropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }
    }
}
