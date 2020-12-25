#include <iostream>
#include <string>

#include "family.pb.h"

using namespace std;
using namespace tutorial;

string SerializeFamilyBytes() {
    Family family;
    family.set_address(10086);

    Person *person = nullptr;
    person = family.add_person_list();
    person->set_age(25);
    person->set_name("John");

    person = family.add_person_list();
    person->set_age(23);
    person->set_name("Lucy");

    person = family.add_person_list();
    person->set_age(2);
    person->set_name("Tony");

    string bytes;
    if (!family.SerializeToString(&bytes)) {
        cout << "SerializeFamilyBytes failed." << endl;
    }
    return bytes;
}

void DeserializeFamilyBytes(const string &bytes) {
    Family family;
    if (!family.ParseFromString(bytes)) {
        cout << "DeserializeFamilyBytes failed." << endl;
        return;
    }

    cout << "Family address " << family.address() << endl;
    int size = family.person_list_size();
    cout << "Family has " << size << " member" << endl;
    for (auto i = 0; i < size; ++i) {
        const Person &p = family.person_list(i);
        cout << "-- name " << p.name() << " age " << p.age() << endl;
    }
}

void TestFamily() {
    DeserializeFamilyBytes(SerializeFamilyBytes());
}

string SerializeAccountBytes(const string &bytes) {
    Account account;
    account.set_province("Guangzhou");

    Family *family = nullptr;
    family = account.add_family_list();
    family->ParseFromString(bytes);

    string res;
    if (!account.SerializeToString(&res)) {
        cout << "SerializeAccountBytes failed." << endl;
    }
    return res;
}

void DeserializeAccountBytes(const string &bytes) {
    Account country;
    if (!country.ParseFromString(bytes)) {
        cout << "DeserializeAccountBytes failed." << endl;
        return;
    }

    cout << "Account province " << country.province() << endl;
    int size = country.family_list_size();

    cout << "Account has " << size << " family" << endl;
    for (int i = 0; i < size; ++i) {
        const Family &f = country.family_list(i);
        cout << "-- Family address " << f.address() << endl;
        cout << "-- Family has " << f.person_list_size() << " member" << endl;
        for (int j = 0; j < f.person_list_size(); ++j) {
            const Person &psn = f.person_list(j);
            cout << "-- -- name " << psn.name() << " age " << psn.age() << endl;
        }
    }
}

void TestAccount() {
    DeserializeAccountBytes(SerializeAccountBytes(SerializeFamilyBytes()));
}

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    TestFamily();
    TestAccount();

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}