#include "../../common/http/httplib.h"
#include "../../common/eqemu_logsys.h"
#include "../../common/platform.h"
#include "../zone.h"
#include "../client.h"
#include "../../common/net/eqstream.h"

extern Zone *zone;

void RunTest(const std::string &test_name, const std::string &expected, const std::string &actual)
{
	if (expected == actual) {
		std::cout << "[✅] " << test_name << " PASSED\n";
	} else {
		std::cerr << "[❌] " << test_name << " FAILED\n";
		std::cerr << "   📌 Expected: " << expected << "\n";
		std::cerr << "   ❌ Got:      " << actual << "\n";
		std::exit(1);
	}
}

void ZoneCLI::DataBuckets(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	uint32 break_length = 50;
	int failed_count = 0;

	LogSys.SilenceConsoleLogging();

	// boot shell zone for testing
	Zone::Bootup(ZoneID("qrg"), 0, false);
	zone->StopShutdownTimer();

	entity_list.Process();
	entity_list.MobProcess();

	LogSys.EnableConsoleLogging();

	std::cout << "===========================================\n";
	std::cout << "Running DataBuckets Tests...\n";
	std::cout << "===========================================\n\n";

	Client *client = new Client();

	/*** FIXED TEST CASES WITH BUCKET RESET ***/

	// 🧪 **Test 1: Basic Key-Value Storage**
	client->DeleteBucket("simple_key"); // Reset
	client->SetBucket("simple_key", "simple_value");
	std::string value = client->GetBucket("simple_key");
	RunTest("Basic Key-Value Set/Get", "simple_value", value);

	// 🧪 **Test 2: Nested Key Storage**
	client->DeleteBucket("nested");
	client->SetBucket("nested.test1", "value1");
	client->SetBucket("nested.test2", "value2");
	value = client->GetBucket("nested");
	RunTest("Nested Key Set/Get", R"({"test1":"value1","test2":"value2"})", value);

	// 🧪 **Test 3: Prevent Overwriting Objects**
	client->DeleteBucket("nested");
	client->SetBucket("nested.test1.a", "value1");
	client->SetBucket("nested.test2.a", "value2");
	client->SetBucket("nested.test2", "new_value");  // Should be **rejected**
	value = client->GetBucket("nested");
	RunTest("Prevent Overwriting Objects", R"({"test1":{"a":"value1"},"test2":{"a":"value2"}})", value);

	// 🧪 **Test 4: Deleting a Specific Nested Key**
	client->DeleteBucket("nested");
	client->SetBucket("nested.test1", "value1");
	client->SetBucket("nested.test2", "value2");
	client->DeleteBucket("nested.test1");
	value = client->GetBucket("nested");
	RunTest("Delete Nested Key", R"({"test2":"value2"})", value);

	// 🧪 **Test 5: Deleting the Entire Parent Key**
	client->DeleteBucket("nested");
	value = client->GetBucket("nested");
	RunTest("Delete Parent Key", "", value);

	// 🧪 **Test 6: Expiration is Ignored for Nested Keys**
	client->DeleteBucket("exp_test");
	client->SetBucket("exp_test.nested", "data", "S20"); // Expiration ignored
	value = client->GetBucket("exp_test");
	RunTest("Expiration Ignored for Nested Keys", R"({"nested":"data"})", value);

	// 🧪 **Test 7: Cache Behavior**
	client->DeleteBucket("cache_test");
	client->SetBucket("cache_test", "cache_value");
	value = client->GetBucket("cache_test");
	RunTest("Cache Read/Write Consistency", "cache_value", value);

	// 🧪 **Test 8: Ensure Deleting Parent Key Clears Cache**
	client->DeleteBucket("cache_test");
	value = client->GetBucket("cache_test");
	RunTest("Cache Clears on Parent Delete", "", value);

	// 🧪 **Test 9: Setting an Entire JSON Object**
	client->DeleteBucket("full_json");
	client->SetBucket("full_json", R"({"key1":"value1","key2":{"subkey":"subvalue"}})");
	value = client->GetBucket("full_json");
	RunTest("Set and Retrieve Full JSON Structure", R"({"key1":"value1","key2":{"subkey":"subvalue"}})", value);

	// 🧪 **Test 10: Partial Nested Key Deletion within JSON**
	client->DeleteBucket("full_json");
	client->SetBucket("full_json", R"({"key1":"value1","key2":{"subkey":"subvalue"}})");
	client->DeleteBucket("full_json.key2");
	value = client->GetBucket("full_json");
	RunTest("Delete Nested Key within JSON", R"({"key1":"value1"})", value);

	// 🧪 **Test 11: Ensure Object Protection on Overwrite Attempt**
	client->DeleteBucket("complex");
	client->SetBucket("complex.nested.obj1", "data1");
	client->SetBucket("complex.nested.obj2", "data2");
	client->SetBucket("complex.nested", "overwrite_attempt"); // Should be rejected
	value = client->GetBucket("complex");
	RunTest("Ensure Object Protection on Overwrite Attempt", R"({"nested":{"obj1":"data1","obj2":"data2"}})", value);

	// 🧪 **Test 12: Deleting Non-Existent Key Doesn't Break Existing Data**
	client->DeleteBucket("complex");
	client->SetBucket("complex.nested.obj1", "data1");
	client->SetBucket("complex.nested.obj2", "data2");
	client->DeleteBucket("does_not_exist");  // Should do nothing
	value = client->GetBucket("complex");
	RunTest("Deleting Non-Existent Key Doesn't Break Existing Data", R"({"nested":{"obj1":"data1","obj2":"data2"}})", value);

	// 🧪 **Test 13: Get nested key value one level up **
	client->DeleteBucket("complex");
	client->SetBucket("complex.nested.obj1", "data1");
	client->SetBucket("complex.nested.obj2", "data2");
	value = client->GetBucket("complex.nested");
	RunTest("Get nested key value", R"({"obj1":"data1","obj2":"data2"})", value);

	// 🧪 **Test 13: Get nested key value deep **
	client->DeleteBucket("complex");
	client->SetBucket("complex.nested.obj1", "data1");
	client->SetBucket("complex.nested.obj2", "data2");
	value = client->GetBucket("complex.nested.obj2");
	RunTest("Get nested key value deep", R"(data2)", value);

	/*** ADDITIONAL TEST CASES ***/

	// 🧪 **Test 1: Retrieve Nested Key from Plain String**
	client->DeleteBucket("plain_string");
	client->SetBucket("plain_string", "some_value");
	value = client->GetBucket("plain_string.nested");
	RunTest("Retrieve Nested Key from Plain String", "", value);

	// 🧪 **Test 2: Store and Retrieve JSON Array**
	client->DeleteBucket("json_array");
	client->SetBucket("json_array", R"(["item1", "item2"])");
	value = client->GetBucket("json_array");
	RunTest("Store and Retrieve JSON Array", R"(["item1", "item2"])", value);

//	// 🧪 **Test 3: Prevent Overwriting Array with Object**
//	client->DeleteBucket("json_array");
//	client->SetBucket("json_array", R"(["item1", "item2"])");
//	client->SetBucket("json_array.item", "new_value"); // Should be rejected
//	value = client->GetBucket("json_array");
//	RunTest("Prevent Overwriting Array with Object", R"(["item1", "item2"])", value);

	// 🧪 **Test 4: Retrieve Non-Existent Nested Key**
	client->DeleteBucket("nested_partial");
	client->SetBucket("nested_partial.level1", R"({"exists": "yes"})");
	value = client->GetBucket("nested_partial.level1.non_existent");
	RunTest("Retrieve Non-Existent Nested Key", "", value);

	// 🧪 **Test 5: Overwriting Parent Key Deletes Children**
	client->DeleteBucket("nested_override");
	client->SetBucket("nested_override.child", "data");
	client->SetBucket("nested_override", "new_parent_value"); // Should remove `child`
	value = client->GetBucket("nested_override");
	RunTest("Overwriting Parent Key Deletes Children", "new_parent_value", value);

	// 🧪 **Test 6: Store and Retrieve Empty JSON Object**
	client->DeleteBucket("empty_json");
	client->SetBucket("empty_json", R"({})");
	value = client->GetBucket("empty_json");
	RunTest("Store and Retrieve Empty JSON Object", R"({})", value);

	// 🧪 **Test 7: Store and Retrieve JSON String**
	client->DeleteBucket("json_string");
	client->SetBucket("json_string", R"("this is a string")");
	value = client->GetBucket("json_string");
	RunTest("Store and Retrieve JSON String", R"("this is a string")", value);

	// 🧪 **Test 8: Deeply Nested Key Retrieval**
	client->DeleteBucket("deep_nested");
	client->SetBucket("deep_nested.level1.level2.level3.level4.level5", "final_value");
	value = client->GetBucket("deep_nested.level1.level2.level3.level4.level5");
	RunTest("Deeply Nested Key Retrieval", "final_value", value);

	// 🧪 **Test 9: Delete Deep Nested Key Keeps Parent**
//	client->DeleteBucket("deep_nested");
//	client->SetBucket("deep_nested.level1.level2.level3", R"({"key": "value"})");
//	client->DeleteBucket("deep_nested.level1.level2.level3.key");
//	value = client->GetBucket("deep_nested.level1.level2.level3");
//	RunTest("Delete Deep Nested Key Keeps Parent", "{}", value);

	std::cout << "\n===========================================\n";
	std::cout << "✅ All DataBucket Tests Completed!\n";
	std::cout << "===========================================\n";
}
