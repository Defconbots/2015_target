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
        public BindableCollection<IMainScreenTabItem> Tabs { get; set; }

        public ShellViewModel(IEnumerable<IMainScreenTabItem> tabs, IEventAggregator eventAggregator)
        {
            DisplayName = "DEFCONBOTS CONTEST CONTROLLER";

            Tabs = new BindableCollection<IMainScreenTabItem>();

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
