#include<string>
#include<iostream>
#include<fstream>
#include<vector>
#include<ctime>
#include<sstream>
#include<iomanip>
#include<functional>
#include <cstdlib>

/*
MediaTracker.cpp
A personal CLI app for tracking different types of media
Saves/loads from a key=value flat file (MediaListSaveFile.txt)
Time: 30hrs

Current Features: Add media(one line, prompt each field), List Media, 
                  Search Media, Sorting(by rating + ViewDate(highest + lowest)),
                  Edit Media, Delete Media, Save To CSV(for easy copying)

TODO:
      
Future Features: Be able to cancel out of prompts and go back to menu, Search by multiple things at once

PROBLEMS: input handling
        
*/
//returns "YYYY-MM-DD" format
std::string getCurrentDate() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    std::ostringstream oss;
    oss << std::setfill('0')
        << (now->tm_year + 1900) << "-"
        << std::setw(2) << (now->tm_mon + 1) << "-"
        << std::setw(2) << now->tm_mday;

    return oss.str(); // e.g. "2026-03-29"
}

const std::string SAVE_FILE = "MediaListSaveFile.txt";
const int MAX_NAME_LENGTH = 45;
const int MAX_RATING = 10;
const size_t MIN_SOURCE_LENGTH = 1;
const size_t MAX_SOURCE_LENGTH = 15;
const std::string MIN_VALID_YEAR = "2022";
const std::string CURRENT_YEAR = getCurrentDate().substr(0, 4);

enum class MediaType{ UNKNOWN, SHOW, MANGA, MOVIE, COUNT };
enum class Status { UNKNOWN, COMPLETED, IN_PROGRESS, PLANNED, DROPPED, COUNT };
struct Media{
    std::string name = "";
    MediaType type = MediaType::UNKNOWN;
    int rating = -1;
    Status completionStatus = Status::UNKNOWN;
    int amountViewed = -1;
    std::string dateAdded = getCurrentDate();
    std::string dateLastViewed = getCurrentDate();
    std::string source = "";
};

struct MenuOption{
    std::string label;
    std::function<void()> action;
};

struct MediaTypeInfo{
    MediaType Type;
    int inputCode;
    std::string label;
};

struct StatusInfo{
    Status status;
    int inputCode;
    std::string label;
};

const std::vector<MediaTypeInfo> MEDIA_TYPE_TABLE = {
    { MediaType::SHOW,  1, "SHOW"  },
    { MediaType::MANGA, 2, "MANGA" },
    { MediaType::MOVIE, 3, "MOVIE" },
};

const std::vector<StatusInfo> STATUS_TABLE = {
    { Status::COMPLETED, 1, "COMPLETED" },
    { Status::IN_PROGRESS, 2, "IN_PROGRESS" },
    { Status::PLANNED, 3, "PLANNED" },
    { Status::DROPPED, 4, "DROPPED" },
};

std::string mediaTypeToString(MediaType type) {
    for(const MediaTypeInfo& entry : MEDIA_TYPE_TABLE)
        if(entry.Type == type) return entry.label;
    return "UNKNOWN";
}

MediaType stringToMediaType(const std::string& str) {
    for(const MediaTypeInfo& entry : MEDIA_TYPE_TABLE)
        if(entry.label == str) return entry.Type;
    return MediaType::UNKNOWN;  
}

std::string statusToString(Status status){
    for(const StatusInfo& entry : STATUS_TABLE)
        if(entry.status == status) return entry.label;
    return "UNKNOWN";
}

Status stringToStatus(const std::string& str) {
    for(const StatusInfo& entry : STATUS_TABLE)
        if(entry.label == str) return entry.status;
    return Status::UNKNOWN;
}

void printHeader(const std::string& title){
    std::cout << '\n' << std::string(30, '=') << '\n'
                      << title << '\n'
                      << std::string(30, '=') << "\n\n";
}

//returns prompt selection-1 because menus are displayed as 1-indexed so the
//index of the chosen option in the array will be one less than selection
int promptMenuSelection(const std::string title, std::vector<MenuOption> menuOptions){
    int selection;

    //Print Menu
    printHeader(title);
    std::cout << "Make a selection" << "\n";
    for(int i = 0; i < (int)menuOptions.size(); i++){
        std::cout << i + 1 << ": " << menuOptions[i].label << "\n";
    }

    //Prompt selection
    std::cout << "Selection: ";
    std::cin >> selection;
    std::cin.ignore();
    std::cout << "\n";

    return selection - 1; //-1 because menus are 1-indexed
}

void callMenuAction(const std::string& menuTitle, 
                    const std::vector<MenuOption>& menuOptions){

    int menuSelection = promptMenuSelection(menuTitle, menuOptions);
    bool validSelection = false;

    while(!validSelection){
        if(menuSelection < 0 || menuSelection >= (int)menuOptions.size()){
            std::cout << "RETRY: Invalid Menu Selection" << "\n";
            menuSelection = promptMenuSelection(menuTitle, menuOptions);
        } else {
            validSelection = true;
        }
    }

    menuOptions[menuSelection].action();
}

// String lexicographic comparison works here since years are zero-padded to 4 digits
bool isValidDate(const std::string& date) {
    //expects "YYYY-MM-DD" or "c"
    if (date == "c") return true;
    if (date.size() != 10) return false;

    const std::string YEAR = date.substr(0, 4);
    const std::string MONTH = date.substr(5, 2);
    const std::string DAY = date.substr(8, 2);
    const std::string CURRENT_YEAR = getCurrentDate().substr(0, 4);

    if (date[4] != '-' || date[7] != '-') return false;
    if (YEAR < MIN_VALID_YEAR || YEAR > CURRENT_YEAR) return false; //validate Year
    if (MONTH < "01" || MONTH > "12") return false; //validate Month
    if (DAY < "01" || DAY > "31") return false; //validate Day 
    return true;
}

std::string shortcutDate(const std::string& date){
    if(date == "c")
        return getCurrentDate();
    return date;
}

bool isValidName(std::string input){
    if (input.size() > MAX_NAME_LENGTH) return false;
    return true;
}

bool isValidType(std::string input){
    for(const auto& entry : MEDIA_TYPE_TABLE)
        if(entry.label == input) return true;
    return false;
}
bool isValidType(int input){
    for(const auto& entry : MEDIA_TYPE_TABLE)
        if(entry.inputCode == input) return true;
    return false;
}

bool isValidRating(int input) {
    if (input < 0 || input > MAX_RATING) return false;
    return true;
}

bool isValidStatus(int input){
    for(const auto& entry : STATUS_TABLE)
        if(entry.inputCode == input) return true;
    return false;
}
bool isValidStatus(std::string input){
    for(const auto& entry : STATUS_TABLE)
        if(entry.label == input) return true;
    return false;
}

bool isValidAmountViewed(int input){
    if (input < 0) return false;
    return true;
}

bool isValidSource(std::string input){
    if (input.size() < MIN_SOURCE_LENGTH || 
        input.size() > MAX_SOURCE_LENGTH) return false;
    return true;
}

bool doesNameExist(const std::vector<Media>& mediaList, std::string name){
    for(const auto& media : mediaList){
        if(media.name == name){
            return true;
        }
    }
    return false;
}

int promptIntUntilValid(const std::string& prompt, std::function<bool(int)> validator) {
    int value;
    while(true) {
        std::cout << prompt;
        std::cin >> value;
        std::cin.ignore();
        if(validator(value))
            return value;
        std::cout << "RETRY: Invalid input.\n";
    }
}

std::string promptStringUntilValid(const std::string& prompt, 
                                   std::function<bool(std::string)> validator) {
    std::string value;
    while(true) {
        std::cout << prompt;
        std::getline(std::cin, value);
        if(validator(value))
            return value;
        std::cout << "RETRY: Invalid input.\n";
    }
}

std::string promptName(){
   std::string prompt = "Enter Name(Max " + std::to_string(MAX_NAME_LENGTH) + " characters): ";
   return promptStringUntilValid(prompt, isValidName);
}

MediaType promptType(){
    std::string prompt = "Enter Type(";

    //builds prompt string
    for(int i = 0; i < (int)MEDIA_TYPE_TABLE.size(); i++){
        prompt += std::to_string(MEDIA_TYPE_TABLE[i].inputCode) 
               + " = " + MEDIA_TYPE_TABLE[i].label;
        if(i != (int)MEDIA_TYPE_TABLE.size() - 1)
            prompt += ", ";
    }
    prompt += "): ";

    int typeInput = promptIntUntilValid(prompt, static_cast<bool(*)(int)>(isValidType));
    for(const auto& entry : MEDIA_TYPE_TABLE)
        if(entry.inputCode == typeInput) return entry.Type;
    return MediaType::UNKNOWN;
}

int promptRating(){
    int ratingInput;
    std::string prompt = "Enter Rating(Integers 0 - " + std::to_string(MAX_RATING) + "): ";

    std::cout << prompt;
    std::cin >> ratingInput;
    std::cin.ignore();

    if(isValidRating(ratingInput))
        return ratingInput;

    std::cout << "SKIPPED: Invalid Input." << "\n";
    return -1;
}

Status promptStatus(){
    std::string prompt = "Enter Status(";
    for(int i = 0; i < (int)STATUS_TABLE.size(); i++){
        prompt += std::to_string(STATUS_TABLE[i].inputCode)
               + " = " + STATUS_TABLE[i].label;
        if(i != (int)STATUS_TABLE.size() - 1)
            prompt += ", ";
    }
    prompt += "): ";
    std::cout << prompt;

    int statusInput;
    std::cin >> statusInput;
    std::cin.ignore();
    if(isValidStatus(statusInput))
        for(const auto& entry : STATUS_TABLE)
            if(entry.inputCode == statusInput) return entry.status;

    std::cout << "SKIPPED: Invalid Input." << "\n";
    return Status::UNKNOWN; 
}

int promptAmountViewed(){
    int amountViewedInput;
    std::string prompt = "Enter Amount Viewed(Chapters, Episodes, Minutes): ";

    std::cout << prompt;
    std::cin >> amountViewedInput;
    std::cin.ignore();
    if(isValidAmountViewed(amountViewedInput))
        return amountViewedInput;

    std::cout << "SKIPPED: Invalid Input." << "\n";
    return -1;
}

std::string promptDateLastViewed(){
    std::string dateString;
    std::string prompt = "Enter Date Last Viewed(YYYY-MM-DD)(c = today): ";

    std::cout << prompt;
    std::getline(std::cin, dateString);
    if(isValidDate(dateString))
        return shortcutDate(dateString);

    std::cout << "SKIPPED: Invalid Input." << "\n";
    return "";
}

std::string promptSource(){
    std::string sourceString;
    std::string prompt = "Enter Source(Max " + std::to_string(MAX_SOURCE_LENGTH) + "): ";

    std::cout << prompt;
    std::getline(std::cin, sourceString);
    if(isValidSource(sourceString))
        return sourceString;

    std::cout << "SKIPPED: Invalid Input." << "\n";
    return "";
}

void saveToFile(const std::vector<Media>& mediaList, const std::string& fileName){
    std::ofstream file(fileName);

    if(file.is_open()){
        for(const Media& item : mediaList){
            int intDefault = -1;
            std::string stringDefault = "";

            if(item.name != stringDefault) 
                file << "name=" << item.name << "\n";
            if(item.type != MediaType::UNKNOWN) 
                file << "type=" << mediaTypeToString(item.type) << "\n";
            if(item.rating != intDefault) 
                file << "rating=" << item.rating << "\n";
            if(item.completionStatus != Status::UNKNOWN) 
                file << "status=" << statusToString(item.completionStatus) << "\n";
            if(item.amountViewed != intDefault)
                file << "amountViewed=" << item.amountViewed << "\n";
            if(item.dateAdded != stringDefault)
                file << "dateAdded=" << item.dateAdded << "\n";
            if(item.dateLastViewed != stringDefault)
                file << "dateLastViewed=" << item.dateLastViewed << "\n";
            if(item.source != stringDefault) 
                file << "source=" << item.source << "\n";
            file << "---" << "\n";
        }
    }else {
        std::cout << "ERROR: COULD NOT OPEN SAVE FILE" << "\n";
    }
    file.close();
}

std::string popKey(std::string& line){
    std::string key;

    if(line.find('=') != std::string::npos){
        int endOfKey = line.find('=');
        key = line.substr(0, endOfKey);
        line = line.substr(endOfKey + 1, line.length());
    }else
        key = line;

    return key;
}

void loadFromFile(std::vector<Media>& mediaList, const std::string& fileName){
    std::ifstream file(fileName);
    std::string line;
    std::string key;
    Media m;
    
    if(file.is_open()){
        while(std::getline(file, line)){
            key = popKey(line);
            std::string value = line;

            if(key == "name"){
                m.name = value;
            }else if(key == "type"){
                m.type = stringToMediaType(value);
            }else if(key == "rating"){
                m.rating = stoi(value);
            }else if(key == "status"){
                m.completionStatus= stringToStatus(value);
            }else if(key == "amountViewed"){
                m.amountViewed = stoi(value);
            }else if(key == "dateAdded"){
                m.dateAdded = value;
            }else if(key == "dateLastViewed"){
                m.dateLastViewed = value;
            }else if(key == "source"){
                m.source = value;
            }else if(key == "---"){
                mediaList.push_back(m);
                m = {};
            }else{
                std::cout << "ERROR: File format is invalid" << "\n";
            }
        }
    } else {
        std::cout << "ERROR: Could not open file" << "\n";
    }
    file.close();
}

void addMedia(std::vector<Media>& mediaList){
    Media a;
    
    a.name = promptName();
    a.type = promptType();
    a.source = promptSource();
    a.completionStatus = promptStatus();

    if(a.completionStatus != Status::PLANNED && a.completionStatus != Status::DROPPED){
        a.rating = promptRating();
        a.amountViewed = promptAmountViewed();
        a.dateAdded = getCurrentDate();
        a.dateLastViewed = promptDateLastViewed();
    } else {
        std::cout << "Rating, Amount Viewed, Date Added, and Date Last Viewed have been left at their default values\n";
    }

    std::cout << "\n";
    mediaList.push_back(a);
    saveToFile(mediaList, SAVE_FILE);
}

/*  
Pops the first feild from a line with comma-separated values
returns the field
note: The line is passed by reference so it is mutated   
*/
std::string popFieldFromLine(std::string& line) {
    int endOfField = line.find(',');
    if (endOfField == std::string::npos) {
        std::string field = line;
        line = "";
        return field;
    }
    std::string field = line.substr(0, endOfField);
    line = line.substr(endOfField + 1, line.size());
    return field;
}

/*
Parses a line with comma-seperated values and
Creates a media object from the fields of the line
note: Line must have every field and in correct order

BIG NOTE: this method could obviously use a helper function
but the helper function gets complicated to make and use due to
some fields needing a stoi() call in the validator so I am
choosing not to do it.

In hindsight, I think leaving all input as strings and only 
converting them to their real data types when creating the object or inside validation
functions would be a smarter maintainance choice in the future
*/
void quickAddMedia(std::vector<Media>& mediaList){
    //expected format: field,field,field
    //every field must be present and in order
    std::string mediaString;
    Media m;
    std::string field;
    bool validString = true;

    std::cout << "Input Media String: ";
    std::getline(std::cin, mediaString);
    std::cout << "\n";

    field = popFieldFromLine(mediaString);
    if(!isValidName(field)) validString = false;
    m.name = field;

    field = popFieldFromLine(mediaString);
    if(!isValidType(field)) validString = false;
    m.type = stringToMediaType(field);

    field = popFieldFromLine(mediaString);
    if(!isValidRating(stoi(field))) validString = false;
    m.rating = stoi(field);

    field = popFieldFromLine(mediaString);
    if(!isValidStatus(field)) validString = false;
    m.completionStatus = stringToStatus(field);

    field = popFieldFromLine(mediaString);
    if(!isValidAmountViewed(stoi(field))) validString = false;
    m.amountViewed = stoi(field);

    field = popFieldFromLine(mediaString);
    if(!isValidDate(field)) validString = false;
    m.dateAdded = field;

    field = popFieldFromLine(mediaString);
    if(!isValidDate(field)) validString = false;
    m.dateLastViewed = field;

    field = popFieldFromLine(mediaString);
    if(!isValidSource(field)) validString = false;
    m.source = field;

    if(validString){
        std::cout << "Successfully inputted Media String\n";
        mediaList.push_back(m);
        saveToFile(mediaList, SAVE_FILE);
    }else {
        std::cout << "ERROR: Invalid String\n";
    }
}

std::string defaultInt(int value){
    if(value == -1)
        return "-";
    return std::to_string(value);
}

void printMediaTable(const std::vector<Media>& mediaList){
    //Column sizes are just random numbers that looked nice in the table (except COL1)
    const int COL1 = MAX_NAME_LENGTH + 2, COL2 = 10, COL3 = 8, 
              COL4 = 13, COL5 = 12, COL6 = 8, COL7 = 12, COL8 = 15;
    printHeader("My Media");
    std::cout << std::left << std::setw(COL1) << "TITLE"
                           << std::setw(COL2) << "TYPE"
                           << std::setw(COL3) << "RATING" 
                           << std::setw(COL4) << "STATUS" 
                           << std::setw(COL5) << "ADDED"
                           << std::setw(COL6) << "VIEWED"
                           << std::setw(COL7) << "LAST VIEWED" 
                           << std::setw(COL8) << "SOURCE" 
                           << "\n"
                           << std::string(COL1 + COL2 + COL3 + COL4 + COL5 + COL6 + COL7 + COL8, '-')
                           << "\n";

    for(const Media& item : mediaList){
        std::cout << std::left << std::setw(COL1) << item.name
                               << std::setw(COL2) << mediaTypeToString(item.type)
                               << std::setw(COL3) << defaultInt(item.rating)
                               << std::setw(COL4) << statusToString(item.completionStatus)
                               << std::setw(COL5) << item.dateAdded
                               << std::setw(COL6) << defaultInt(item.amountViewed)
                               << std::setw(COL7) << item.dateLastViewed 
                               << std::setw(COL8) << item.source
                               << '\n';
    }
    std::cout << '\n';
}

bool isValidYear(std::string year){
    if(year < MIN_VALID_YEAR || year > CURRENT_YEAR) return false;
    return true;
}

enum class SearchField{ NAME, STATUS, TYPE, SOURCE, YEAR_ADDED, YEAR_VIEWED, RATING };
std::vector<Media> searchMedia(const std::vector<Media>& mediaList, const SearchField& field){
    std::vector<Media> result;
    
    if(field == SearchField::NAME){
        std::string name = promptStringUntilValid("Title: ", isValidName);
        for(const auto& media : mediaList)
            if (media.name == name)
                result.push_back(media);
    }

    if(field == SearchField::STATUS){
        std::string prompt = "Enter Status(";
        for(int i = 0; i < (int)STATUS_TABLE.size(); i++){
            prompt += STATUS_TABLE[i].label;
            if(i != (int)STATUS_TABLE.size() - 1)
                prompt += ", ";
        }
        prompt += "): ";

        std::string status = promptStringUntilValid(prompt, static_cast<bool(*)(std::string)>(isValidStatus));
        for(const auto& media : mediaList)
            if(media.completionStatus == stringToStatus(status))
                result.push_back(media);
    }

    if(field == SearchField::TYPE){
        std::string prompt = "Enter Type(";
        for(int i = 0; i < (int)MEDIA_TYPE_TABLE.size(); i++){
            prompt += MEDIA_TYPE_TABLE[i].label;
            if(i != (int)MEDIA_TYPE_TABLE.size() - 1)
                prompt += ", ";
        }
        prompt += "): ";

        std::string type = promptStringUntilValid(prompt, static_cast<bool(*)(std::string)>(isValidType));
        for(const auto& media : mediaList)
            if(media.type == stringToMediaType(type))
                result.push_back(media);
    }

    if(field == SearchField::SOURCE){
        std::string prompt = "Enter Source(";
        std::vector<std::string> sources;

        for(const auto& media : mediaList){
            bool sourceFound = false;
            for(const auto& source : sources){
                if(media.source == source) sourceFound = true;
            }
            if(!sourceFound) 
                sources.push_back(media.source);
        }

        for(int i = 0; i < (int)sources.size(); i++){
            prompt += sources[i];
            if (i != (int)sources.size() - 1)
                prompt += ", ";
        }
        prompt += "): ";

        std::string source = promptStringUntilValid(prompt, isValidSource);
        for(const auto& media : mediaList){
            if(media.source == source)
                result.push_back(media);
        }
    }

    if(field == SearchField::YEAR_ADDED){
        std::string prompt = "Enter Year(" + MIN_VALID_YEAR + " - " + CURRENT_YEAR + "): ";

        std::string year = promptStringUntilValid(prompt, isValidYear);
        for(const auto& media : mediaList){
            if(media.dateAdded.substr(0,4) == year) 
                result.push_back(media);
        }
    }

    if(field == SearchField::YEAR_VIEWED){
        std::string prompt = "Enter Year(" + MIN_VALID_YEAR + " - " + CURRENT_YEAR + "): ";

        std::string year = promptStringUntilValid(prompt, isValidYear);
        for(const auto& media : mediaList){
            if(media.dateLastViewed.substr(0,4) == year) 
                result.push_back(media);
        }
    }

    if(field == SearchField::RATING){
        std::string prompt = "Enter Rating(1 - 10): ";
        int searchRating = promptIntUntilValid(prompt, isValidRating);

        for(const auto& media : mediaList){
            if(media.rating == searchRating)
                result.push_back(media);
        }
    }

    return result;
}

//Helper function for searchMedia
std::string getNameTarget() {
    std::string target;

    std::cout << "Enter title: ";
    std::getline(std::cin, target);

    return target;
}

//Helper function for searchMedia
Status getStatusTarget() {
    std::string target;
    std::string prompt = "Enter Status(";
    for(int i = 0; i < (int)STATUS_TABLE.size(); i++){
        prompt += STATUS_TABLE[i].label;
        if(i != (int)STATUS_TABLE.size() - 1)
            prompt += ", ";
    }
    prompt += "): ";

    std::cout << prompt;
    std::cin >> target;
    std::cin.ignore();

    return stringToStatus(target);
}

std::vector<Media> sortMediaByRating(const std::vector<Media>& mediaList){
    std::vector<Media> sortedMedia;
    
    for(int i = 10; i >= 1; i--){
        for(const auto& media: mediaList){
            if(media.rating == i) 
                sortedMedia.push_back(media);
        }
    }

    return sortedMedia;
}

enum class SortDirection { MOST_RECENT, LEAST_RECENT };
std::vector<Media> sortMediaByViewDate(const std::vector<Media>& mediaList, SortDirection comparison) {
    std::vector<Media> sortedMedia;
    bool direction = (comparison == SortDirection::MOST_RECENT);

    for (const auto& oldMedia : mediaList) {
        if (sortedMedia.empty()) {
            sortedMedia.push_back(oldMedia);
            continue;
        }

        bool inserted = false;
        for (int i = 0; i < (int)sortedMedia.size(); i++) {
            bool shouldInsert;
            if(direction)
                shouldInsert = oldMedia.dateLastViewed >= sortedMedia[i].dateLastViewed;
            else
                shouldInsert = oldMedia.dateLastViewed <= sortedMedia[i].dateLastViewed;

            if (shouldInsert) {
                sortedMedia.insert(sortedMedia.begin() + i, oldMedia);
                inserted = true;
                break; 
            }
        }

        if (!inserted) {
            sortedMedia.push_back(oldMedia);
        }
    }
    
    return sortedMedia;
}

std::vector<Media> sortMediaByAmountViewed(std::vector<Media> mediaList){
    for (int i = 1; i < (int)mediaList.size(); ++i) {
        Media key = mediaList[i];
        int j = i - 1;

        while (j >= 0 && mediaList[j].amountViewed < key.amountViewed) {
            mediaList[j + 1] = mediaList[j];
            j--;
        }
        
        mediaList[j + 1] = key;
    }

    return mediaList;
}

std::vector<Media> sortMediaByAlphabet(std::vector<Media> mediaList){
    for (int i = 1; i < (int)mediaList.size(); ++i) {
        Media key = mediaList[i];
        int j = i - 1;

        while (j >= 0 && mediaList[j].name > key.name) {
            mediaList[j + 1] = mediaList[j];
            j--;
        }
        
        mediaList[j + 1] = key;
    }

    return mediaList;
}

enum class EditField{ RATING, STATUS, AMOUNT, VIEWDATE, NAME, SOURCE, TYPE };
void editMedia(std::vector<Media>& mediaList, EditField field){
    std::string targetName = getNameTarget();
    int targetIndex = -1;

    if(!doesNameExist(mediaList, targetName)){
        std::cout << "ERROR: Title Does Not Exists\n";
        return;
    } 

    std::cout << "Found Media!\n";
    for(int i = 0; i < (int)mediaList.size(); i++){
        if(targetName == mediaList[i].name){
            targetIndex = i;
            break;
        }
    }

    if(field == EditField::RATING){
        int newRating = promptIntUntilValid("New Rating: ", isValidRating);
        mediaList[targetIndex].rating = newRating;
    } 

    if(field == EditField::STATUS){
        std::string prompt = "Enter Status(";
        for(int i = 0; i < (int)STATUS_TABLE.size(); i++){
            prompt += STATUS_TABLE[i].label;
            if(i != (int)STATUS_TABLE.size() - 1)
                prompt += ", ";
        }
        prompt += "): ";

        std::string newStatus = promptStringUntilValid(prompt, static_cast<bool(*)(std::string)>(isValidStatus));
        mediaList[targetIndex].completionStatus = stringToStatus(newStatus);
    }

    if(field == EditField::AMOUNT){
        int newAmountViewed = promptIntUntilValid("New Amount Viewed: ", isValidAmountViewed);
        mediaList[targetIndex].amountViewed = newAmountViewed;
    }

    if(field == EditField::VIEWDATE){
        std::string newDate = shortcutDate(promptStringUntilValid("Enter Date(YYYY-MM-DD)(c = today): ", isValidDate));
        mediaList[targetIndex].dateLastViewed = newDate;
    }

    if(field == EditField::NAME){
        std::string newName = promptName();
        mediaList[targetIndex].name = newName;
    }

    if(field == EditField::SOURCE){
        std::string newSource = promptStringUntilValid("Enter Source: ", isValidSource);
        mediaList[targetIndex].source = newSource;
    }

    if(field == EditField::TYPE){
        std::string prompt = "Enter Type(";
        for(int i = 0; i < (int)MEDIA_TYPE_TABLE.size(); i++){
            prompt += MEDIA_TYPE_TABLE[i].label;
            if(i != (int)MEDIA_TYPE_TABLE.size() - 1)
                prompt += ", ";
        }
        prompt += "): ";

        std::string newType = promptStringUntilValid(prompt, static_cast<bool(*)(std::string)>(isValidType));
        mediaList[targetIndex].type = stringToMediaType(newType);
    }

    saveToFile(mediaList, SAVE_FILE);
    std::cout << "Edit Successful!\n";
}

void deleteMedia(std::vector<Media>& mediaList){
    std::string targetName = getNameTarget();
    int targetIndex = -1;

    if(!doesNameExist(mediaList, targetName)){
        std::cout << "ERROR: Title Does Not Exists\n";
        return;
    } 

    std::cout << "Found Media!\n";
    for(int i = 0; i < (int)mediaList.size(); i++){
        if(targetName == mediaList[i].name){
            targetIndex = i;
            break;
        }
    }

    mediaList.erase(mediaList.begin() + targetIndex);
    saveToFile(mediaList, SAVE_FILE);
}

void deleteLastAddedMedia(std::vector<Media>& mediaList){
    std::string mediaTitle = mediaList[mediaList.size() - 1].name;
    
    mediaList.pop_back();
    std::cout << "\"" << mediaTitle << "\"" << " was removed from the list.\n";
    
    saveToFile(mediaList, SAVE_FILE);
}

//CSV for easy copy and pasting later
void saveCSVToFile(const std::vector<Media>& mediaList, const std::string& fileName){
    std::ofstream file(fileName);

    if (file.is_open()){
        file << "Title,Media Type,Personal Rating,CompletionStatus,AmountViewed,DateAdded,DateLastViewed,Source\n";
        for(const Media& item : mediaList){
            file << item.name << ','
                 << mediaTypeToString(item.type) << ','
                 << item.rating << ','
                 << statusToString(item.completionStatus) << ','
                 << item.amountViewed << ','
                 << item.dateAdded << ','
                 << item.dateLastViewed << ','
                 << item.source
                 << '\n';
        }
    } else {
        std::cout << "ERROR: COULD NOT OPEN SAVE FILE" << "\n";
        return;
    }
    file.close();

    std::cout << "Saved to CSV!";
}

void returnToMainMenu(){
    std::cout << "Returning To Main Menu . . .\n";
}

void runDeleteMenu(std::vector<Media>& mediaList){
    std::vector<MenuOption> deleteMenuOptions = {
        {"Exit To Main",      [&]() { returnToMainMenu(); }},
        {"Delete by Title",   [&]() { deleteMedia(mediaList); }},
        {"Delete Last Added", [&]() { deleteLastAddedMedia(mediaList); }},
    };

    callMenuAction("DeleteMenu", deleteMenuOptions);
}

void runEditMenu(std::vector<Media>& mediaList){
    std::vector<MenuOption> editMenuOptions = {
        {"Exit To Main",          [&]() { returnToMainMenu(); }},
        {"Edit Title",            [&]() { editMedia(mediaList, EditField::NAME); }},
        {"Edit Type",             [&]() { editMedia(mediaList, EditField::TYPE); }},
        {"Edit Rating",           [&]() { editMedia(mediaList, EditField::RATING); }},
        {"Edit Status",           [&]() { editMedia(mediaList, EditField::STATUS); }},
        {"Edit Amount Viewed",    [&]() { editMedia(mediaList, EditField::AMOUNT); }},
        {"Edit Date Last Viewed", [&]() { editMedia(mediaList, EditField::VIEWDATE); }},
        {"Edit Source",           [&]() { editMedia(mediaList, EditField::SOURCE); }},
        
    }; 

    callMenuAction("Edit Menu", editMenuOptions);
}

void runSortMenu(const std::vector<Media>& mediaList){
    std::vector<MenuOption> sortMenuOptions = {
        {"Return To Main",                [&]() { returnToMainMenu(); }},
        {"Sort By Highest Rating",        [&]() { printMediaTable(sortMediaByRating(mediaList)); }},
        {"Sort By Most Recent View Date", [&]() { printMediaTable(sortMediaByViewDate(mediaList, SortDirection::MOST_RECENT)); }},
        {"Sort By Oldest View Date",      [&]() { printMediaTable(sortMediaByViewDate(mediaList, SortDirection::LEAST_RECENT)); }},
        {"Sort By Amount Viewed",         [&]() { printMediaTable(sortMediaByAmountViewed(mediaList)); }},
        {"Sort By Alphabet",              [&]() { printMediaTable(sortMediaByAlphabet(mediaList)); }},
    };

    callMenuAction("Sort Menu", sortMenuOptions);
}

void runSearchMenu(const std::vector<Media>& mediaList){
    std::vector<MenuOption> searchMenuOptions = {
        {"Return To Main",        [&]() { returnToMainMenu(); }},
        {"Search by Title",       [&]() { printMediaTable(searchMedia(mediaList, SearchField::NAME)); }},
        {"Search by Type",        [&]() { printMediaTable(searchMedia(mediaList, SearchField::TYPE)); }},
        {"Search by Rating",      [&]() { printMediaTable(searchMedia(mediaList, SearchField::RATING)); }},
        {"Search by Status",      [&]() { printMediaTable(searchMedia(mediaList, SearchField::STATUS)); }},
        {"Search by Year Added",  [&]() { printMediaTable(searchMedia(mediaList, SearchField::YEAR_ADDED)); }},
        {"Search by Year Viewed", [&]() { printMediaTable(searchMedia(mediaList, SearchField::YEAR_VIEWED)); }},
        {"Search by Source",      [&]() { printMediaTable(searchMedia(mediaList, SearchField::SOURCE)); }},
        
    };

    callMenuAction("Search Menu", searchMenuOptions);
}

void runAddMenu(std::vector<Media>& mediaList){
    std::vector<MenuOption> addMenuOptions = {
        {"Return To Main", [&]() { returnToMainMenu(); }},
        {"Add each field", [&]() { addMedia(mediaList); }},
        {"Quick Add",      [&]() { quickAddMedia(mediaList); }},
    };

    callMenuAction("Add Media Menu", addMenuOptions);
}


void resizeConsoleWindow(int height, int width){
    std::cout << "\x1b[8;" << height << ";" << width << "t";
}

int main(){

    int WINDOW_HEIGHT = 35;
    int WINDOW_WIDTH = 140;
    bool isRunning = true;
    std::vector<Media> myMedia;

    resizeConsoleWindow(WINDOW_HEIGHT, WINDOW_WIDTH); 

    std::vector<MenuOption> mainMenuOptions = {
        {"Exit",         [&]() { std::cout << "Closing . . ." << "\n"; isRunning = false; }},
        {"Add Media",    [&]() { runAddMenu(myMedia); }},
        {"List Media",   [&]() { printMediaTable(myMedia); }},
        {"Search Media", [&]() { runSearchMenu(myMedia); }},
        {"Sort Media",   [&]() { runSortMenu(myMedia); }},
        {"Edit Media",   [&]() { runEditMenu(myMedia); }},
        {"Delete Media", [&]() { runDeleteMenu(myMedia); }},
        {"Write To CSV", [&]() { saveCSVToFile(myMedia, "MediaListCSV.txt"); }},
    };

    loadFromFile(myMedia, SAVE_FILE);
    
    while(isRunning){
       callMenuAction("Main Menu", mainMenuOptions);
    }

    saveToFile(myMedia, SAVE_FILE);

    return 0;
} 