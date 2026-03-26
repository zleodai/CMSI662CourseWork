import java.util.Arrays;

public final class SecureStackTester {
    private static int testsRun = 0;
    private static int testsFailed = 0;

    private SecureStackTester() {
    }

    public static void main(final String[] args) {
        testCreateAndDestroy();
        testInvalidCreateRequests();
        testPushAndPopLifo();
        testNullArgumentHandling();
        testDeepCopyBehavior();
        testReturnedValueIsDefensiveCopy();
        testStringEdgeCases();
        testDestroyCleansRemainingItems();
        testUseAfterDestroyProtection();

        System.out.println();
        System.out.println("Tests run: " + testsRun);
        System.out.println("Tests failed: " + testsFailed);

        if (testsFailed != 0) {
            System.exit(1);
        }
    }

    private static void recordResult(final boolean condition, final String testName) {
        testsRun++;
        if (condition) {
            System.out.println("[PASS] " + testName);
            return;
        }

        testsFailed++;
        System.out.println("[FAIL] " + testName);
    }

    private static void testCreateAndDestroy() {
        SecureStack.CreateResult createResult = SecureStack.create(3);
        SecureStack.BooleanResult emptyResult;
        SecureStack.BooleanResult fullResult;
        SecureStack.OperationResult destroyResult;

        recordResult(createResult.success() && createResult.stack() != null,
                "create returns a success result and non-null stack");
        if (!createResult.success() || createResult.stack() == null) {
            return;
        }

        emptyResult = createResult.stack().isEmpty();
        fullResult = createResult.stack().isFull();

        recordResult(emptyResult.success() && emptyResult.value(),
                "isEmpty succeeds for a new stack");
        recordResult(fullResult.success() && !fullResult.value(),
                "isFull succeeds for a new stack");

        destroyResult = createResult.stack().destroy();
        recordResult(destroyResult.success(), "destroy succeeds for a valid stack");
    }

    private static void testInvalidCreateRequests() {
        SecureStack.CreateResult zeroResult = SecureStack.create(0);
        SecureStack.CreateResult negativeResult = SecureStack.create(-1);

        recordResult(!zeroResult.success() && zeroResult.code() == SecureStack.StatusCode.INVALID_ARGUMENT,
                "create rejects zero capacity");
        recordResult(zeroResult.message() != null && !zeroResult.message().isEmpty(),
                "create provides an error message for zero capacity");
        recordResult(!negativeResult.success() && negativeResult.code() == SecureStack.StatusCode.INVALID_ARGUMENT,
                "create rejects negative capacity");
    }

    private static void testPushAndPopLifo() {
        SecureStack.CreateResult createResult = SecureStack.create(3);
        SecureStack.OperationResult pushResult;
        SecureStack.BooleanResult fullResult;
        SecureStack.BooleanResult emptyResult;
        SecureStack.ValueResult topResult;
        SecureStack.ValueResult middleResult;
        SecureStack.ValueResult bottomResult;
        SecureStack.ValueResult underflowResult;

        recordResult(createResult.success() && createResult.stack() != null,
                "create succeeds for LIFO test");
        if (!createResult.success() || createResult.stack() == null) {
            return;
        }

        pushResult = createResult.stack().push("first".toCharArray());
        recordResult(pushResult.success(), "push accepts first element");
        pushResult = createResult.stack().push("second".toCharArray());
        recordResult(pushResult.success(), "push accepts second element");
        pushResult = createResult.stack().push("third".toCharArray());
        recordResult(pushResult.success(), "push accepts third element");

        fullResult = createResult.stack().isFull();
        recordResult(fullResult.success() && fullResult.value(),
                "stack reports full when capacity reached");

        pushResult = createResult.stack().push("fourth".toCharArray());
        recordResult(!pushResult.success() && pushResult.code() == SecureStack.StatusCode.FULL,
                "push rejects overflow");

        topResult = createResult.stack().pop();
        middleResult = createResult.stack().pop();
        bottomResult = createResult.stack().pop();

        recordResult(topResult.success() && Arrays.equals(topResult.value(), "third".toCharArray()),
                "pop returns most recent item first");
        recordResult(middleResult.success() && Arrays.equals(middleResult.value(), "second".toCharArray()),
                "pop preserves LIFO order");
        recordResult(bottomResult.success() && Arrays.equals(bottomResult.value(), "first".toCharArray()),
                "pop returns earliest item last");

        emptyResult = createResult.stack().isEmpty();
        recordResult(emptyResult.success() && emptyResult.value(),
                "stack is empty after all elements popped");

        underflowResult = createResult.stack().pop();
        recordResult(!underflowResult.success() && underflowResult.code() == SecureStack.StatusCode.EMPTY,
                "pop returns an empty-stack response on underflow");

        createResult.stack().destroy();
    }

    private static void testNullArgumentHandling() {
        SecureStack.CreateResult createResult = SecureStack.create(1);
        SecureStack.OperationResult pushResult;

        recordResult(createResult.success() && createResult.stack() != null,
                "create succeeds for null input test");
        if (!createResult.success() || createResult.stack() == null) {
            return;
        }

        pushResult = createResult.stack().push(null);
        recordResult(!pushResult.success() && pushResult.code() == SecureStack.StatusCode.INVALID_ARGUMENT,
                "push rejects null input");
        recordResult(pushResult.message() != null && !pushResult.message().isEmpty(),
                "push provides an error message for null input");

        createResult.stack().destroy();
    }

    private static void testDeepCopyBehavior() {
        SecureStack.CreateResult createResult = SecureStack.create(2);
        SecureStack.OperationResult pushResult;
        SecureStack.ValueResult popResult;
        char[] input = "alpha".toCharArray();

        recordResult(createResult.success() && createResult.stack() != null,
                "create succeeds for deep-copy test");
        if (!createResult.success() || createResult.stack() == null) {
            return;
        }

        pushResult = createResult.stack().push(input);
        recordResult(pushResult.success(), "push accepts mutable caller buffer");
        input[0] = 'X';

        popResult = createResult.stack().pop();
        recordResult(popResult.success() && popResult.value() != null,
                "pop returns stored value after source buffer mutation");
        recordResult(popResult.success() && Arrays.equals(popResult.value(), "alpha".toCharArray()),
                "stack stores a deep copy rather than caller-owned memory");
        recordResult(popResult.success() && popResult.value() != input,
                "pop returns a defensive copy distinct from the caller buffer");

        if (popResult.value() != null) {
            Arrays.fill(popResult.value(), '\0');
        }
        createResult.stack().destroy();
    }

    private static void testReturnedValueIsDefensiveCopy() {
        SecureStack.CreateResult createResult = SecureStack.create(2);
        SecureStack.OperationResult pushResult;
        SecureStack.ValueResult firstPop;
        SecureStack.ValueResult secondPop;

        recordResult(createResult.success() && createResult.stack() != null,
                "create succeeds for defensive-copy pop test");
        if (!createResult.success() || createResult.stack() == null) {
            return;
        }

        pushResult = createResult.stack().push("copy-check".toCharArray());
        recordResult(pushResult.success(), "push succeeds for defensive-copy pop test");

        firstPop = createResult.stack().pop();
        recordResult(firstPop.success() && firstPop.value() != null,
                "first pop succeeds for defensive-copy pop test");
        if (!firstPop.success() || firstPop.value() == null) {
            createResult.stack().destroy();
            return;
        }

        firstPop.value()[0] = 'X';
        pushResult = createResult.stack().push("copy-check".toCharArray());
        recordResult(pushResult.success(), "push succeeds after caller mutates returned value");

        secondPop = createResult.stack().pop();
        recordResult(secondPop.success() && Arrays.equals(secondPop.value(), "copy-check".toCharArray()),
                "caller mutation of returned value does not affect future stack values");

        Arrays.fill(firstPop.value(), '\0');
        if (secondPop.value() != null) {
            Arrays.fill(secondPop.value(), '\0');
        }
        createResult.stack().destroy();
    }

    private static void testStringEdgeCases() {
        SecureStack.CreateResult createResult = SecureStack.create(2);
        SecureStack.OperationResult pushResult;
        SecureStack.ValueResult longResult;
        SecureStack.ValueResult emptyResult;
        char[] longInput = new char[1024];

        Arrays.fill(longInput, 'A');

        recordResult(createResult.success() && createResult.stack() != null,
                "create succeeds for string edge-case test");
        if (!createResult.success() || createResult.stack() == null) {
            return;
        }

        pushResult = createResult.stack().push(new char[0]);
        recordResult(pushResult.success(), "push accepts empty char array");
        pushResult = createResult.stack().push(longInput);
        recordResult(pushResult.success(), "push accepts long char array");

        longResult = createResult.stack().pop();
        emptyResult = createResult.stack().pop();

        recordResult(longResult.success() && Arrays.equals(longResult.value(), longInput),
                "pop preserves long array contents");
        recordResult(emptyResult.success() && emptyResult.value() != null && emptyResult.value().length == 0,
                "pop preserves empty array contents");

        if (longResult.value() != null) {
            Arrays.fill(longResult.value(), '\0');
        }
        if (emptyResult.value() != null) {
            Arrays.fill(emptyResult.value(), '\0');
        }
        Arrays.fill(longInput, '\0');
        createResult.stack().destroy();
    }

    private static void testDestroyCleansRemainingItems() {
        SecureStack.CreateResult createResult = SecureStack.create(2);
        SecureStack.OperationResult pushResult;
        SecureStack.OperationResult destroyResult;

        recordResult(createResult.success() && createResult.stack() != null,
                "create succeeds for destroy cleanup test");
        if (!createResult.success() || createResult.stack() == null) {
            return;
        }

        pushResult = createResult.stack().push("secret-1".toCharArray());
        recordResult(pushResult.success(), "push accepts first cleanup value");
        pushResult = createResult.stack().push("secret-2".toCharArray());
        recordResult(pushResult.success(), "push accepts second cleanup value");

        destroyResult = createResult.stack().destroy();
        recordResult(destroyResult.success(), "destroy releases stack with unpopped items");
    }

    private static void testUseAfterDestroyProtection() {
        SecureStack.CreateResult createResult = SecureStack.create(1);
        SecureStack.OperationResult destroyResult;
        SecureStack.OperationResult pushResult;
        SecureStack.ValueResult popResult;
        SecureStack.BooleanResult emptyResult;
        SecureStack.BooleanResult fullResult;

        recordResult(createResult.success() && createResult.stack() != null,
                "create succeeds for use-after-destroy test");
        if (!createResult.success() || createResult.stack() == null) {
            return;
        }

        destroyResult = createResult.stack().destroy();
        recordResult(destroyResult.success(), "destroy succeeds before use-after-destroy checks");

        pushResult = createResult.stack().push("x".toCharArray());
        popResult = createResult.stack().pop();
        emptyResult = createResult.stack().isEmpty();
        fullResult = createResult.stack().isFull();

        recordResult(!pushResult.success() && pushResult.code() == SecureStack.StatusCode.INVALID_STATE,
                "push rejects use after destroy");
        recordResult(!popResult.success() && popResult.code() == SecureStack.StatusCode.INVALID_STATE,
                "pop rejects use after destroy");
        recordResult(!emptyResult.success() && emptyResult.code() == SecureStack.StatusCode.INVALID_STATE,
                "isEmpty rejects use after destroy");
        recordResult(!fullResult.success() && fullResult.code() == SecureStack.StatusCode.INVALID_STATE,
                "isFull rejects use after destroy");
    }
}
