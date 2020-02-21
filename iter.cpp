vector<int> matchingStrings(vector<string> strings, vector<string> queries) {
vector<int> sc(queries.size());
int i;
vector<string>::iterator it, jt;

    for (i=0; i<sc.size(); i++) sc[i]=0;

    for( it = queries.begin(), i=0; it !=  queries.end(); it++, i++) {
        for (jt = strings.begin(); jt != strings.end(); jt++) {
            if (*it == *jt) sc[i] += 1;
        }
    }
    return sc;
}
