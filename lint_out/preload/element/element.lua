-- Export basic functions
createElement = createElement;
isElement = isElement;

function destroyElement(elementToDestroy)
    elementToDestroy.destroy();
    return true;
end

function getAttachedElements(theElement)
    return theElement.getAttached();
end

function getElementAttachedOffsets(theElement)
    return theElement.getAttachmentOffsets();
end

function getElementCollisionsEnabled(theElement)
    return theElement.collEnabled;
end

function getElementAttachedTo(theElement)
    return theElement.getAttachedTo();
end

function getElementBoundingBox(theElement)
    return -1, -1, -1, 1, 1, 1;
end

function getElementByID(id, index)
    if (index == nil) then index = 0; end
    
    local curIndex = 0;
    
    for m,n in ipairs(elements) do
        if (n.getID() == id) then
            if (curIndex == index) then
                return n;
            end
            
            curIndex = curIndex + 1;
        end
    end
    
    return false;
end

function getElementChild(parent, index)
    index = index + 1;
    return parent.getChildren()[index];
end

function getElementChildren(parent, theType)
    local children = parent.getChildren();
    
    if (theType == nil) then return children; end;
    
    local newList = {};
    
    for m,n in ipairs(children) do
        if (n.getType() == theType) then
            table.insert(newList, n);
        end
    end
    
    return newList;
end

function getElementChildrenCount(parent)
    return #parent.getChildren();
end

function getElementData(theElement, key, inherit)
    if (inherit == nil) then inherit = true; end
    
    return theElement.data[key];
end

function getElementDimension(theElement)
    return theElement.dimension;
end

function getElementDistanceFromCentryOfMassToBaseOfModel(theElement)
    return 1;
end

function getElementHealth(theElement)
    return theElement.getHealth();
end

function getElementID(theElement)
    return theElement.getID();
end

function getElementInterior(theElement)
    return theElement.interior;
end

function getElementParent(theElement)
    return theElement.getParent();
end

function getElementPosition(theElement)
    return theElement.x, theElement.y, theElement.z;
end

function getElementRadius(theElement)
    return 1.5;
end

-- Forward declaration.
local _insertChildrenElementsByType;

function _insertChildrenElementsByType(list, parent, theType)
    local children = parent.getChildren();
    
    for m,n in ipairs(children) do
        if (n.getType() == theType) then
            table.insert(list, n);
        end
    
        _insertChildrenElementsByType(list, n, theType);
    end
end

function getElementsByType(theType, startat, streamedIn)
    if (startat == nil) then startat = root; end
    if (streamedIn == nil) then streamedIn = false; end
    
    -- All elements are streamed in by default.
    
    local newList = {};
    
    if (startat == root) then
        for m,n in ipairs(elements) do
            if (n.getType() == theType) then
                table.insert(newList, n);
            end
        end
    else
        _insertChildrenElementsByType(newList, startat);
    end
    
    return newList;
end

function getElementsWithinColShape(shape, elemType)
    -- We do not do colshapes on MTA:Lua Interpreter
    return {};
end

function getElementType(theElement)
    return theElement.getType();
end

function getElementVelocity(theElement)
    return 0, 0, 0;
end

function getRootElement()
    return root;
end

function isElementAttached(theElement)
    return not (theElement.getAttachedTo() == false);
end

function isElementLocal(theElement)
    -- MTA:Lua Interpreter only supports local elements.
    return true;
end

function isElementOnScreen(theElement)
    -- We cannot render elements, so they cannot be on screen.
    return false;
end

function isElementStreamable(theElement)
    -- No element is streamable
    return false;
end

function isElementStreamedIn(theElement)
    -- Every element is streamed in though.
    return true;
end

function isElementWithinColShape(theElement, theShape)
    -- We do not do colshape checks.
    return false;
end

function setElementAlpha(theElement, alpha)
    theElement.alpha = alpha;
    return true;
end

function setElementCollisionsEnabled(theElement, enabled)
    theElement.collEnabled = enabled;
    return true;
end

function setElementData(theElement, key, value, synchronize)
    if (synchronize == nil) then synchronize = true; end
    
    -- We do not synchronize data.
    
    theElement.data[key] = value;
    return true;
end

function setElementDimension(theElement, dimension)
    theElement.dimension = dimension;
    return true;
end

function setElementDoubleSided(theElement, enable)
    theElement.doublesided = enable;
    return true;
end

function setElementHealth(theElement, newHealth)
    -- Cannot damage elements.
    return false;
end

function setElementID(theElement, name)
    theElement.setID(name);
    return true;
end

function setElementInterior(theElement, interior, x, y, z)
    theElement.interior = interior;
    theElement.x, theElement.y, theElement.z = x, y, z;
    return true;
end

function setElementParent(theElement, parent)
    return theElement.setParent(parent);
end

function setElementPosition(theElement, x, y, z, warp)
    if (warp == nil) then warp = true; end
    
    theElement.x, theElement.y, theElement.z = x, y, z;
    return true;
end

function setElementStreamable(theElement, streamable)
    -- No elements can be streamable.
    return false;
end

function setElementVelocity(theElement, x, y, z)
    -- Elements cannot have velocity.
    return false;
end

function detachElements(theElement, theAttachToElement)
    local attachedTo = theElement.getAttachedTo();

    if not (theAttachToElement == nil) then
        if not (attachedTo == theAttachToElement) then
            return false;
        end
    end
    
    attachedTo.detachElement(theElement);
    return true;
end

function getElementColShape(theElement)
    -- The interpreter does not support col shapes.
    return false;
end

function getElementMatrix(theElement)
    -- We do not support element matrices.
    return false;
end

function getElementModel(theElement)
    -- We do not support element models.
    return false;
end

function getElementRotation(theElement)
    return theElement.rotX, theElement.rotY, theElement.rotZ;
end

function isElementInWater(theElement)
    -- There is no water!
    return false;
end

function isElementSyncer(theElement)
    -- Yes, we sync everything ourselves.
    return true;
end

function setElementModel(theElement, model)
    -- No model support for now.
    return false;
end

function setElementRotation(theElement, rotX, rotY, rotZ)
    theElement.rotX, theElement.rotY, theElement.rotZ = rotX, rotY, rotZ;
    return true;
end

function isElementCollidableWith(theElement, withElement)
    -- There are no collision checks!
    return false;
end

function setElementCollidableWith(theElement, withElement, enabled)
    -- No, we do not support this.
    return false;
end

function isElementDoubleSided(theElement)
    return theElement.doublesided;
end

function setLowLODElement(theElement, lowLODElement)
    -- We ain't rendering anything, so screw LODs.
    return false;
end

function getLowLODElement(theElement)
    return false;
end

function isElementLowLOD(theElement)
    return false;
end

function attachElementToElement(theElement, theAttachToElement, xPosOffset, yPosOffset, zPosOffset, xRotOffset, yRotOffset, zRotOffset)
    if (xPosOffset == nil) then xPosOffset = 0; end
    if (yPosOffset == nil) then yPosOffset = 0; end
    if (zPosOffset == nil) then zPosOffset = 0; end
    if (xRotOffset == nil) then xRotOffset = 0; end
    if (yRotOffset == nil) then yRotOffset = 0; end
    if (zRotOffset == nil) then zRotOffset = 0; end

    return theAttachToElement.attachElement(theElement, xPosOffset, yPosOffset, zPosOffset, xRotOffset, yRotOffset, zRotOffset);
end

function detachElementFromElement(theElement, theAttachToElement)
    local attachedTo = theElement.getAttachedTo();
    
    if not (theAttachToElement == nil) then
        if not (attachedTo == theAttachToElement) then
            return false;
        end
    end
    
    attachedTo.detachElement(theElement);
    return true;
end

function isElementFrozen(theElement)
    return theElement.frozen;
end

function setElementFrozen(theElement, freezeStatus)
    theElement.frozen = freezeStatus;
    return true;
end

function setElementMatrix(theElement, theMatrix)
    -- No matrices for you!
    return false;
end