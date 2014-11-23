using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Threading.Tasks;

namespace TargetControl.Models
{
    public class TeamMember // this should probably be a view model if you intend on editing it in the UI
    {
        public string Name { get; set; }
        public string Email { get; set; }
        public string Website { get; set; }
        public string PhoneNumber { get; set; }
    }
    public class Team
    {
        private const int string_display_length = 25;
        public string Name { get; set; }
        public string HitId { get; set; }
        public List<TeamMember> Members { get; set; }
        public List<int> QualScores { get; set; }
        public List<int> FinalScores { get; set; }
    }
}
