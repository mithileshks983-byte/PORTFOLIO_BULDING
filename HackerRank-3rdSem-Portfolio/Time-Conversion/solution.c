#include <bits/stdc++.h>

using namespace std;

string timeConversion(string s) {
    string hour_str = s.substr(0, 2);
    int hour = stoi(hour_str);
    string rest_of_time = s.substr(2, 6);
    string am_pm = s.substr(8, 2);

    if (am_pm == "AM") {
        if (hour == 12) {
            hour_str = "00";
        }
    } else {
        if (hour != 12) {
            hour += 12;
            hour_str = to_string(hour);
        }
    }

    return hour_str + rest_of_time;
}

int main()
{
    ofstream fout(getenv("OUTPUT_PATH"));

    string s;
    getline(cin, s);

    string result = timeConversion(s);

    fout << result << "\n";

    fout.close();

    return 0;
}