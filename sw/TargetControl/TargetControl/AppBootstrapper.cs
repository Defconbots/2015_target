using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

using Caliburn.Micro;

using SimpleInjector;

namespace TargetControl
{
    class AppBootstrapper : BootstrapperBase
    {
        private Container _container = new Container();

        public AppBootstrapper()
        {
            Initialize();
        }

        protected override void Configure()
        {
            _container.Register<IWindowManager, WindowManager>();
            _container.RegisterSingle<IEventAggregator, EventAggregator>();

            _container.RegisterAll(typeof(IMainScreenTabItem), new[]
                {
                    typeof(TeamsViewModel),
                    typeof(ContestViewModel),
                    typeof(ManualControlViewModel),
                    typeof(TerminalViewModel)
                });
            
            _container.Verify();
        }
        
        protected override object GetInstance(Type serviceType, string key)
        {
            return _container.GetInstance(serviceType);
        }

        protected override IEnumerable<object> GetAllInstances(Type serviceType)
        {
            return _container.GetAllInstances(serviceType);
        }

        protected override void BuildUp(object instance)
        {
            var registration = _container.GetRegistration(instance.GetType(), true);
            registration.Registration.InitializeInstance(instance);
        }

        protected override void OnStartup(object sender, StartupEventArgs e)
        {
            DisplayRootViewFor<ShellViewModel>();
        }
    }
}
