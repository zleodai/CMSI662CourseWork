import java.util.Arrays;

public final class SecureStack {
    public enum StatusCode {
        OK,
        INVALID_ARGUMENT,
        INVALID_STATE,
        ALLOCATION_FAILURE,
        FULL,
        EMPTY
    }

    public static final class OperationResult {
        private final boolean success;
        private final StatusCode code;
        private final String message;

        private OperationResult(final boolean success, final StatusCode code, final String message) {
            this.success = success;
            this.code = code;
            this.message = message;
        }

        public boolean success() {
            return success;
        }

        public StatusCode code() {
            return code;
        }

        public String message() {
            return message;
        }
    }

    public static final class CreateResult {
        private final boolean success;
        private final StatusCode code;
        private final String message;
        private final SecureStack stack;

        private CreateResult(final boolean success,
                             final StatusCode code,
                             final String message,
                             final SecureStack stack) {
            this.success = success;
            this.code = code;
            this.message = message;
            this.stack = stack;
        }

        public boolean success() {
            return success;
        }

        public StatusCode code() {
            return code;
        }

        public String message() {
            return message;
        }

        public SecureStack stack() {
            return stack;
        }
    }

    public static final class BooleanResult {
        private final boolean success;
        private final StatusCode code;
        private final String message;
        private final boolean value;

        private BooleanResult(final boolean success,
                              final StatusCode code,
                              final String message,
                              final boolean value) {
            this.success = success;
            this.code = code;
            this.message = message;
            this.value = value;
        }

        public boolean success() {
            return success;
        }

        public StatusCode code() {
            return code;
        }

        public String message() {
            return message;
        }

        public boolean value() {
            return value;
        }
    }

    public static final class ValueResult {
        private final boolean success;
        private final StatusCode code;
        private final String message;
        private final char[] value;

        private ValueResult(final boolean success,
                            final StatusCode code,
                            final String message,
                            final char[] value) {
            this.success = success;
            this.code = code;
            this.message = message;
            this.value = value;
        }

        public boolean success() {
            return success;
        }

        public StatusCode code() {
            return code;
        }

        public String message() {
            return message;
        }

        public char[] value() {
            return value;
        }
    }

    private final int capacity;
    private int count;
    private final char[][] items;
    private boolean destroyed;

    private SecureStack(final int capacity) {
        this.capacity = capacity;
        this.count = 0;
        this.items = new char[capacity][];
        this.destroyed = false;
    }

    public static CreateResult create(final int capacity) {
        if (capacity <= 0) {
            return new CreateResult(false, StatusCode.INVALID_ARGUMENT,
                    "Stack capacity must be greater than zero.", null);
        }

        try {
            return new CreateResult(true, StatusCode.OK, "Stack created successfully.", new SecureStack(capacity));
        } catch (OutOfMemoryError error) {
            return new CreateResult(false, StatusCode.ALLOCATION_FAILURE,
                    "Failed to allocate stack storage.", null);
        }
    }

    public OperationResult destroy() {
        int index;

        if (destroyed) {
            return new OperationResult(false, StatusCode.INVALID_STATE, "Stack has already been destroyed.");
        }

        for (index = 0; index < count; index++) {
            if (items[index] != null) {
                Arrays.fill(items[index], '\0');
                items[index] = null;
            }
        }

        count = 0;
        destroyed = true;
        return new OperationResult(true, StatusCode.OK, "Stack destroyed successfully.");
    }

    public OperationResult push(final char[] value) {
        if (destroyed) {
            return new OperationResult(false, StatusCode.INVALID_STATE, "Stack has been destroyed.");
        }

        if (value == null) {
            return new OperationResult(false, StatusCode.INVALID_ARGUMENT, "Input value is null.");
        }

        if (count >= capacity) {
            return new OperationResult(false, StatusCode.FULL, "Stack is full.");
        }

        try {
            items[count] = Arrays.copyOf(value, value.length);
        } catch (OutOfMemoryError error) {
            return new OperationResult(false, StatusCode.ALLOCATION_FAILURE,
                    "Failed to allocate a defensive copy of the input value.");
        }

        count++;
        return new OperationResult(true, StatusCode.OK, "Value pushed successfully.");
    }

    public ValueResult pop() {
        char[] storedValue;
        char[] returnedValue;

        if (destroyed) {
            return new ValueResult(false, StatusCode.INVALID_STATE, "Stack has been destroyed.", null);
        }

        if (count == 0) {
            return new ValueResult(false, StatusCode.EMPTY, "Stack is empty.", null);
        }

        count--;
        storedValue = items[count];
        items[count] = null;

        try {
            returnedValue = Arrays.copyOf(storedValue, storedValue.length);
        } catch (OutOfMemoryError error) {
            Arrays.fill(storedValue, '\0');
            return new ValueResult(false, StatusCode.ALLOCATION_FAILURE,
                    "Failed to allocate a defensive copy of the popped value.", null);
        }

        Arrays.fill(storedValue, '\0');
        return new ValueResult(true, StatusCode.OK, "Value popped successfully.", returnedValue);
    }

    public BooleanResult isEmpty() {
        if (destroyed) {
            return new BooleanResult(false, StatusCode.INVALID_STATE, "Stack has been destroyed.", false);
        }

        return new BooleanResult(true, StatusCode.OK, "Stack emptiness evaluated successfully.", count == 0);
    }

    public BooleanResult isFull() {
        if (destroyed) {
            return new BooleanResult(false, StatusCode.INVALID_STATE, "Stack has been destroyed.", false);
        }

        return new BooleanResult(true, StatusCode.OK, "Stack fullness evaluated successfully.", count == capacity);
    }
}
