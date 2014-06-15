-- FileSystem functionality that should be tested before implementation into the C++ framework.
-- This is created since development in Lua is easier than in C++.
-- Routines here should represent bare prototype code, unconnected.

-- The idea of a slice is simple, getting the implementation you want is not.
-- It is an accomplishment to get your concepts stable into code.

-- Routine that should test the reading slice collision and what it returns.
local function createSlice(startPoint, endPoint)
    local slice = {};
    local comparisonOnly = true;
    
    function slice.getStartPoint()
        return startPoint;
    end
    
    function slice.getEndPoint()
        return endPoint;
    end
    
    function slice.intersectWith(otherSlice)
        local sliceStartA = startPoint;
        local sliceEndA = endPoint;
        local sliceStartB = otherSlice.getStartPoint();
        local sliceEndB = otherSlice.getEndPoint();
    
        -- We do not support negated slices.
        if ( sliceStartA <= sliceEndA ) and ( sliceStartB <= sliceEndB ) then
            if (comparisonOnly == true) then
                -- An implementation of the slice logic using comparisons only.
                -- We hide the offset operations (using minus operators) in the comparators.
                -- This is the cleanest implementation as it does not rely on arithmetic logic.
                -- The only requirement for this logic to work is that the number type supports comparison.
                if (sliceStartA == sliceStartB) and (sliceEndA == sliceEndB) then
                    return "equal";
                end
                
                if (sliceStartB >= sliceStartA) and (sliceEndB <= sliceEndA) then
                    return "enclosing";
                end
                
                if (sliceStartB <= sliceStartA) and (sliceEndB >= sliceEndA) then
                    return "inside";
                end
                
                if (sliceStartB < sliceStartA) and ( (sliceEndB >= sliceStartA) and (sliceEndB <= sliceEndA) ) then
                    return "intersect_start";
                end
                
                if (sliceEndB > sliceEndA) and ( (sliceStartB >= sliceStartA) and (sliceStartB <= sliceEndA) ) then
                    return "intersect_end";
                end
                
                if (sliceStartB < sliceStartA) and (sliceEndB < sliceStartA) then
                    return "floating_end";
                end
                
                if (sliceStartB > sliceEndA) and (sliceEndB > sliceEndA) then
                    return "floating_start";
                end
            else
                local function testSamplePointOnLineAFromLineB(paramB)
                    return
                    (
                        ( sliceStartB - sliceStartA )
                        +
                        ( sliceEndB - sliceStartB ) * paramB
                    )
                    /
                    ( sliceEndA - sliceStartA );
                end
                
                local function testSamplePointOnLineBFromLineA(paramA)
                    return
                    (
                        ( sliceStartA - sliceStartB )
                        +
                        ( sliceEndA - sliceStartA ) * paramA
                    )
                    /
                    ( sliceEndB - sliceStartB );
                end
                
                local function isInside(num)
                    return ( num >= 0 ) and ( num <= 1 );
                end
            
                -- Since we want logic that works on unsigned systems, we must make sure that
                -- there are no negative interim results possible.
                if ( sliceStartB >= sliceStartA ) then
                    -- Get collision parameters based on line A.
                    local colBStartPointOnA = testSamplePointOnLineAFromLineB( 0 );
                    local colBEndPointOnA = testSamplePointOnLineAFromLineB( 1 );
                    
                    -- Check what intersection we got.
                    -- We are computing in the floating point realm, where 0 is the start point and 1 is the end point.
                    -- To support FileSystem unsigned wide integers, we will have to couple the parameters.
                    if (colBStartPointOnA == 0) and (colBEndPointOnA == 1) then
                        return "equal";
                    end
                    
                    if (colBStartPointOnA >= 0) and (colBEndPointOnA <= 1) then
                        return "enclosing";
                    end
                    
                    if (colBStartPointOnA <= 0) and (colBEndPointOnA >= 1) then
                        return "inside";
                    end
                    
                    if (colBStartPointOnA < 0) and ( isInside(colBEndPointOnA) ) then
                        return "intersect_start";
                    end
                    
                    if (colBEndPointOnA > 1) and ( isInside(colBStartPointOnA) ) then
                        return "intersect_end";
                    end
                    
                    if (colBStartPointOnA < 0) and (colBEndPointOnA < 0) then
                        return "floating_end";
                    end
                    
                    if (colBStartPointOnA > 1) and (colBEndPointOnA > 1) then
                        return "floating_start";
                    end
                    
                    -- This should never happen.
                    return
                        "unknown (colBStartPointOnA: " .. colBStartPointOnA ..
                        ", colBEndPointOnA: " .. colBEndPointOnA .. ")";
                else
                    -- We must calculate results based on line B.
                    local colAStartPointOnB = testSamplePointOnLineBFromLineA( 0 );
                    local colAEndPointOnB = testSamplePointOnLineBFromLineA( 1 );
                    
                    -- Check what intersection we got.
                    -- We are computing in the floating point realm, where 0 is the start point and 1 is the end point.
                    -- To support FileSystem unsigned wide integers, we will have to couple the parameters.
                    if (colAStartPointOnB == 0) and (colAEndPointOnB == 1) then
                        return "equal";
                    end
                    
                    if (colAStartPointOnB >= 0) and (colAEndPointOnB <= 1) then
                        return "inside";
                    end
                    
                    if (colAStartPointOnB <= 0) and (colAEndPointOnB >= 1) then
                        return "enclosing";
                    end
                    
                    if (colAStartPointOnB < 0) and ( isInside(colAEndPointOnB) ) then
                        return "intersect_end";
                    end
                    
                    if (colAEndPointOnB > 1) and ( isInside(colAStartPointOnB) ) then
                        return "intersect_start";
                    end
                    
                    if (colAStartPointOnB < 0) and (colAEndPointOnB < 0) then
                        return "floating_start";
                    end
                    
                    if (colAStartPointOnB > 1) and (colAEndPointOnB > 1) then
                        return "floating_end";
                    end
                    
                    -- This should never happen.
                    return
                        "unknown (colAStartPointOnB: " .. colAStartPointOnB ..
                        ", colAEndPointOnB: " .. colAEndPointOnB .. ")";
                end
            
                --[[
                    slicePos = sliceStartA + (sliceEndA - sliceStartA) * a
                    slicePos = sliceStartB + (sliceEndB - sliceStartB) * b
                    
                    a = ( slicePos - sliceStartA ) / ( sliceEndA - sliceStartA );
                    b = ( slicePos - sliceStartB ) / ( sliceEndB - sliceStartB );
                    
                    a = ( sliceStartB + ( sliceEndB - sliceStartB ) * b - sliceStartA ) / ( sliceEndA - sliceStartA );
                    b = ( sliceStartA + ( sliceEndA - sliceStartA ) * a - sliceStartB ) / ( sliceEndB - sliceStartB );
                    
                    a = ( ( sliceStartB - sliceStartA ) + ( sliceEndB - sliceStartB ) * b ) / ( sliceEndA - sliceStartA );
                    b = ( ( sliceStartA - sliceStartB ) + ( sliceEndA - sliceStartA ) * a ) / ( sliceEndB - sliceStartB );
                    
                    ; the rest is comparison logic.
                    ; math can be optimized to comparison logic only.
                ]]
            end
        end
        
        return "unknown";
    end
    
    return slice;
end

-- A simple test that should check whether the slice logic works at all.
function slicetest()
    local success, errorObj = pcall(
        function()
            local sliceOne = createSlice( 100, 201 );
            local sliceTwo = createSlice( 200, 240 );

            local colResult = sliceOne.intersectWith( sliceTwo );
            
            print( "the collision result (slice one -> slice two) is " .. sliceOne.intersectWith( sliceTwo ) );
            print( "the collision result (slice two -> slice one) is " .. sliceTwo.intersectWith( sliceOne ) );
        end
    );
    
    if not (success) then
        print( "slice logic is faulty" );
        
        if ( errorObj ) then
            print( tostring( errorObj ) );
        end
    end
end

-- Routine that should prove proper functionality of the slice logic.
-- Execute this when changes to the slice logic have been made; to assert its functionality.
function sliceunittest()
    -- The implementation of the slice we should use.
    local createSlice = createSlice;

    -- Table to keep track of the tests that have failed.
    -- If it stays empty, the slice implementation is stable.
    local failedTests = {};

    local function perftest(testName, sliceOne, sliceTwo, expResult)
        local intersect_result = sliceOne.intersectWith( sliceTwo );
    
        print(
            testName .. ": " .. intersect_result ..
            " -> " .. expResult
        );
        
        -- Remember all the failed tests.
        if not (intersect_result == expResult) then
            table.insert( failedTests, testName );
        end
    end

    -- Test basic intersections.
    do
        -- Test intersection at start.
        perftest(
            "Basic Intersect Start",
            createSlice( 0, 10 ),
            createSlice( 5, 15 ),
            "intersect_end"
        );
        
        -- Test intersection at end.
        perftest(
            "Basic Intersect End",
            createSlice( 5, 15 ),
            createSlice( 0, 10 ),
            "intersect_start"
        );
        
        -- Test safe inclusion.
        perftest(
            "Basic Inclusion",
            createSlice( 5, 15 ),
            createSlice( 0, 20 ),
            "inside"
        );
        
        -- Test safe encapsulation.
        perftest(
            "Basic Encapsulation",
            createSlice( 0, 20 ),
            createSlice( 5, 15 ),
            "enclosing"
        );
        
        -- Test floating.
        perftest(
            "Basic Before Floating",
            createSlice( 0, 100 ),
            createSlice( 200, 300 ),
            "floating_start"
        );
        
        perftest(
            "Basic After Floating",
            createSlice( 200, 300 ),
            createSlice( 0, 100 ),
            "floating_end"
        );
    end
    
    -- Test advanced bounds checking.
    do
        local sampleSlice = createSlice( 100, 200 );
    
        -- Test starting bound integrity.
        perftest(
            "Advanced Start Outside Touch",
            sampleSlice,
            createSlice( 0, sampleSlice.getStartPoint() - 1 ),
            "floating_end"
        );
        
        perftest(
            "Advanced Start Inside Touch",
            sampleSlice,
            createSlice( sampleSlice.getStartPoint(), sampleSlice.getStartPoint() + 10 ),
            "enclosing"
        );
        
        perftest(
            "Advanced End Outside Touch",
            sampleSlice,
            createSlice( sampleSlice.getEndPoint() + 1, sampleSlice.getEndPoint() + 2 ),
            "floating_start"
        );
        
        perftest(
            "Advanced End Inside Touch",
            sampleSlice,
            createSlice( sampleSlice.getEndPoint() - 1, sampleSlice.getEndPoint() ),
            "enclosing"
        );
        
        perftest(
            "Advanced Equality Intersect",
            sampleSlice,
            sampleSlice,
            "equal"
        );
        
        perftest(
            "Advanced Undefined Behavior Test",
            createSlice( 100, 10 ),
            createSlice( 0, 50 ),
            "unknown"
        );
    end
    
    -- Report to the runtime whether we failed and what failed.
    print( "-------------" );
    
    if ( #failedTests == 0 ) then
        -- If this is printed, the slice logic works perfectly.
        print( "all tests successful" );
    else
        for m,n in ipairs( failedTests ) do
            print( "failed test '" .. n .. "'" );
        end
    end
end